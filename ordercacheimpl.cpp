#include "ordercacheimpl.hpp"

#include <algorithm>

void OrderCacheImpl::addOrder(Order order) { m_orders.push_back(std::move(order)); }

void OrderCacheImpl::cancelOrder(const std::string& orderId)
{
    std::scoped_lock lock(m_remove_order_mutex);
    m_orders.erase(
        std::find_if(m_orders.begin(), m_orders.end(), [&orderId](auto& order) { return orderId == order.orderId(); }));
}

void OrderCacheImpl::cancelOrdersForUser(const std::string& user)
{
    std::scoped_lock lock(m_remove_order_mutex);
    auto it { m_orders.begin() };
    while (it != m_orders.end()) {
        it = m_orders.erase(std::find_if(m_orders.begin(), m_orders.end(), [&user](auto& order) { return user == order.user(); }));
    }
}

void OrderCacheImpl::cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty)
{
    std::scoped_lock lock(m_remove_order_mutex);
    m_orders.erase(std::remove_if(m_orders.begin(), m_orders.end(), [&securityId, minQty](const Order& order) {
        return securityId == order.securityId() && order.qty() >= minQty;
    }),
        m_orders.end());
}

std::pair<std::vector<size_t>, std::vector<size_t>> OrderCacheImpl::splitOrdersIdsByOrderType(std::vector<size_t>& ordersIds)
{
    auto it = std::stable_partition(ordersIds.begin(), ordersIds.end(), [&orders = m_orders](auto elem) { return orders[elem].side() == "Sell"; });
    std::vector<size_t> buyOrdersIds(it, ordersIds.end());

    ordersIds.erase(it, ordersIds.end());

    return { ordersIds, buyOrdersIds };
}

std::pair<std::vector<size_t>, std::vector<size_t>> OrderCacheImpl::getOrdersIdsFor(const std::string& securityId)
{
    std::vector<size_t> ordersIds;
    for (auto it = m_orders.begin(); it != m_orders.end();) {
        it = std::find_if(it, m_orders.end(), [&securityId](const Order& order) { return order.securityId() == securityId; });
        if (it != m_orders.end()) {
            ordersIds.emplace_back(std::distance(m_orders.begin(), it));
            it++;
        }
    }
    return splitOrdersIdsByOrderType(ordersIds);
}

unsigned int OrderCacheImpl::getMatchingSizeForSecurity(const std::string& securityId)
{
    // adding orders doesn't seem to require a locking, if new order would be added after getOrdersFor() call then
    // won't be calculated within this getMatchingSizeForSecurity() call
    // removing is more complicated scenario cause it invalidates indexes
    std::scoped_lock lock(m_remove_order_mutex);
    auto [sell_orders, buy_orders] = getOrdersIdsFor(securityId);
    if (!sell_orders.size() || !buy_orders.size()) {
        return 0;
    }

    unsigned int out = 0;
    for (auto& sell_order : sell_orders) {
        for (auto& buy_order : buy_orders) {
            if (m_orders[sell_order].company() != m_orders[buy_order].company()) {
                unsigned int transaction_qty = 0;
                if (m_orders[sell_order].qty() >= m_orders[buy_order].qty()) {
                    transaction_qty = m_orders[buy_order].qty();
                    m_orders[sell_order] -= transaction_qty;
                    m_orders[buy_order] -= transaction_qty;
                } else {
                    transaction_qty = m_orders[sell_order].qty();
                    m_orders[buy_order] -= transaction_qty;
                    m_orders[sell_order] -= transaction_qty;
                }

                out += transaction_qty;

                if (m_orders[sell_order].qty() == 0) {
                    break;
                }
            }
        }
    }

    return out;
    // in such implementation (more API desing) it might be good to schedule some kind of cleaner execution here
    // to get rid of all orders where qty == 0 now, we probably don't want to bother those in next iteration
    // it could be done here since getMatchingSizeForSecurity() is not const yet, probably we woud be interested in good
    // performacne here, on the other hand dox didn't say anything about that.
}

std::pair<std::vector<Order>, std::vector<Order>> OrderCacheImpl::splitOrdersByType(std::vector<Order>& orders)
{
    auto it = std::stable_partition(orders.begin(), orders.end(), [](const auto& order) { return order.side() == "Sell"; });
    std::vector<Order> buyOrders;
    std::move(it, orders.end(), std::back_inserter(buyOrders));

    orders.erase(it, orders.end());

    return { orders, buyOrders };
}

std::pair<std::vector<Order>, std::vector<Order>> OrderCacheImpl::getOrdersFor(const std::string& securityId)
{
    std::unique_lock ulock(m_remove_order_mutex);
    std::vector<Order> ordersIds;
    for (auto it = m_orders.begin(); it != m_orders.end();) {
        it = std::find_if(it, m_orders.end(), [&securityId](const Order& order) { return order.securityId() == securityId; });
        if (it != m_orders.end()) {
            ordersIds.push_back(std::move(*it));
            m_orders.erase(it);
            it = m_orders.begin();
        }
    }
    ulock.unlock();
    return splitOrdersByType(ordersIds);
}

unsigned int OrderCacheImpl::getMatchingSizeForSecurity2(const std::string& securityId)
{
    auto [sell_orders, buy_orders] = getOrdersFor(securityId);

    unsigned int out = 0;
    if (sell_orders.size() and buy_orders.size()) {

        for (auto& sell_order : sell_orders) {
            for (auto& buy_order : buy_orders) {
                if (sell_order.company() != buy_order.company()) {
                    unsigned int transaction_qty = 0;
                    if (sell_order.qty() >= buy_order.qty()) {
                        transaction_qty = buy_order.qty();
                        sell_order -= transaction_qty;
                        buy_order -= transaction_qty;
                    } else {
                        transaction_qty = sell_order.qty();
                        buy_order -= transaction_qty;
                        sell_order -= transaction_qty;
                    }

                    out += transaction_qty;

                    if (sell_order.qty() == 0) {
                        break;
                    }
                }
            }
        }
    }
    insertOrders(sell_orders);
    insertOrders(buy_orders);

    return out;
    // this implementation does cleanup by moving elements in getOrdersFor() function
}

void OrderCacheImpl::insertOrders(std::vector<Order>& orders)
{
    for (auto& order : orders) {
        if (order.qty() > 0) {
            m_orders.push_back(std::move(order));
        }
    }
}

std::vector<Order> OrderCacheImpl::getAllOrders() const
{
    return m_orders;
}
