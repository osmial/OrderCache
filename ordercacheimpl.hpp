#ifndef ORDERCACHEIMPL1_HPP
#define ORDERCACHEIMPL1_HPP

#include "ordercacheinterface.hpp"

#include <mutex>
#include <vector>

class OrderCacheImpl : public OrderCacheInterface {
public:
    ~OrderCacheImpl() = default;

    // OrderCacheInterface interface
    void addOrder(Order order) override;
    void cancelOrder(const std::string& orderId) override;
    void cancelOrdersForUser(const std::string& user) override;
    void cancelOrdersForSecIdWithMinimumQty(const std::string& securityId, unsigned int minQty) override;

    // This particular impl will not cancel/update matching orders yet I left the comment in code where cleanup shall be done if needed.
    unsigned int getMatchingSizeForSecurity(const std::string& securityId) override;

    // additional impl providing the same functionality but changes the state of the object, the container with orders
    // will have updated orders and removed fully matched (qty == 0) after each call to that function but thanks to that
    // the scope of locks is decreased significantly, unfortunatelly provided example data is too small to measure the difference
    // yet it was fun task to implement that.
    unsigned int getMatchingSizeForSecurity2(const std::string& securityId) override;

    std::vector<Order> getAllOrders() const override;
    /*notes:
     * The interface could be improved - adding, canceling orders could return an information if operation succeded.
     * getMatchingSizeForSecurity is not marked as const which makes the interface bit ambigous cause docs are not sharing more details about the state of orders when matched,
     * shall those be updated or cancelled/removed from the list of pending orders ?
     *
     * This impl utilizes a std::vector as a container for orders yet if the performance would be the key factor probably unordered_multimap would be better,
     * but the problem with unordered_multimap is that the requested API doesn't express which property of Order shall be used as a Key,
     * i.e. it looks like the implememtation should focus on getMatchingSizeForSecurity's performance so securityId could be used as a Key
     * but if there would be many calls to cancelOrder() or cancelOrdersForUser() then it would anyway require traversing through whole map slowing down the execution.
     * Ofc it could always be implemented in other ways yet it would require better understanding of the requirements so I didn't spend time on that.
     *
     * Additional note, this impl is not designed to work correctly with concurrent calls to both getMatchingSizeForSecurity() and getMatchingSizeForSecurity2()
     * It may work yet I didn't test and thought about that during implementing those. It's kind of use One-or-Another API extention.
     */
private:
    // <sell_orders, buy_orders>
    std::pair<std::vector<size_t>, std::vector<size_t>> getOrdersIdsFor(const std::string& securityId);
    std::pair<std::vector<size_t>, std::vector<size_t>> splitOrdersIdsByOrderType(std::vector<size_t>& ordersIds);

    std::pair<std::vector<Order>, std::vector<Order>> getOrdersFor(const std::string& securityId);
    std::pair<std::vector<Order>, std::vector<Order>> splitOrdersByType(std::vector<Order>& orders);

    void insertOrders(std::vector<Order>& orders);

    std::vector<Order> m_orders;
    std::mutex m_remove_order_mutex;
};

#endif // ORDERCACHEIMPL1_HPP
