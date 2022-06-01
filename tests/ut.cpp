#include <gtest/gtest.h>
#include <iostream>
#include <memory>

#include "../ordercacheimpl.hpp"

std::ostream& operator<<(std::ostream& os, const Order& order)
{
    os << "OrderId: " << order.orderId() << " SecurityId: " << order.securityId() << " Side: " << order.side() << " Quantity: " << order.qty() << " User: " << order.user() << " Company: " << order.company() << "\n";
    return os;
}

bool operator==(const Order& lhr, const Order& rhr)
{
    return lhr.company() == rhr.company() && lhr.orderId() == rhr.orderId() && lhr.qty() == rhr.qty() && lhr.securityId() == rhr.securityId() && lhr.side() == rhr.side() && lhr.user() == rhr.user();
}

class OrderCacheInterfaceTests : public ::testing::Test {

protected:
    void SetUp() override
    {
        m_orderCacheInterfacePtr = std::make_unique<OrderCacheImpl>();
    }

    std::unique_ptr<OrderCacheInterface> m_orderCacheInterfacePtr;
};

TEST_F(OrderCacheInterfaceTests, AddOrder_Succeeds)
{
    const Order order { "OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA" };
    m_orderCacheInterfacePtr->addOrder(order);
    auto orders { m_orderCacheInterfacePtr->getAllOrders() };
    EXPECT_EQ(orders.size(), 1);
    EXPECT_EQ(orders[0], order);
}

// add test for AddOrder_Fails when trying to add two orders with the same orderId

TEST_F(OrderCacheInterfaceTests, CancelOrder_SingleOrderList_Succeeds)
{
    static const std::string orderId { "OrdId1" };
    m_orderCacheInterfacePtr->addOrder({ orderId, "SecId1", "Buy", 1000, "User1", "CompanyA" });
    auto orders { m_orderCacheInterfacePtr->getAllOrders() };
    EXPECT_EQ(orders.size(), 1);
    m_orderCacheInterfacePtr->cancelOrder(orderId);
    orders = m_orderCacheInterfacePtr->getAllOrders();
    EXPECT_EQ(orders.size(), 0);
}

TEST_F(OrderCacheInterfaceTests, CancelOrder_MultiOrderList_Succeeds)
{
    const std::string orderId { "OrdId2" }; // static?
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId2", "SecId1", "Buy", 1000, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId3", "SecId1", "Buy", 1000, "User1", "CompanyA" });
    auto orders { m_orderCacheInterfacePtr->getAllOrders() };
    EXPECT_EQ(orders.size(), 3);
    m_orderCacheInterfacePtr->cancelOrder(orderId);
    orders = m_orderCacheInterfacePtr->getAllOrders();
    EXPECT_EQ(orders.size(), 2);
    for (auto& order : orders) {
        EXPECT_TRUE(order.orderId() != orderId);
    }
}

TEST_F(OrderCacheInterfaceTests, CancelOrdersForUser_SingleOrderList_Succeeds)
{
    static const std::string user { "User1" };
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId1", "Buy", 1000, user, "CompanyA" });
    auto orders { m_orderCacheInterfacePtr->getAllOrders() };
    EXPECT_EQ(orders.size(), 1);
    m_orderCacheInterfacePtr->cancelOrdersForUser(user);
    orders = m_orderCacheInterfacePtr->getAllOrders();
    EXPECT_EQ(orders.size(), 0);
}

TEST_F(OrderCacheInterfaceTests, CancelOrdersForUser_ManyOrdersOneUser_Succeeds)
{
    static const std::string user { "User1" };
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId1", "Buy", 1000, user, "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId2", "SecId1", "Sell", 1000, user, "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId3", "SecId1", "Sell", 1000, user, "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId4", "SecId1", "Buy", 1000, user, "CompanyA" });
    auto orders { m_orderCacheInterfacePtr->getAllOrders() };
    EXPECT_EQ(orders.size(), 4);
    m_orderCacheInterfacePtr->cancelOrdersForUser(user);
    orders = m_orderCacheInterfacePtr->getAllOrders();
    EXPECT_EQ(orders.size(), 0);
}

TEST_F(OrderCacheInterfaceTests, CancelOrdersForUser_ManyOrdersFromManyUsers_Succeeds)
{
    static const std::string user { "User2" };
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId1", "Buy", 1000, user, "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId2", "SecId1", "Sell", 1000, user, "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId3", "SecId1", "Sell", 1000, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId4", "SecId1", "Buy", 1000, "User3", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId5", "SecId1", "Buy", 1000, "User3", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId6", "SecId1", "Sell", 1000, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId7", "SecId1", "Sell", 1000, user, "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId8", "SecId1", "Buy", 1000, user, "CompanyA" });
    auto orders { m_orderCacheInterfacePtr->getAllOrders() };
    EXPECT_EQ(orders.size(), 8);
    m_orderCacheInterfacePtr->cancelOrdersForUser(user);
    orders = m_orderCacheInterfacePtr->getAllOrders();
    EXPECT_EQ(orders.size(), 4);
    for (auto& order : orders) {
        EXPECT_TRUE(order.user() != user);
    }
}

TEST_F(OrderCacheInterfaceTests, CancelOrdersForSecIdWithMinimumQty_SingleOrderList_Succeeds)
{
    static const std::string secId { "SecId1" };
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", secId, "Buy", 1000, "User1", "CompanyA" });
    auto orders { m_orderCacheInterfacePtr->getAllOrders() };
    EXPECT_EQ(orders.size(), 1);
    m_orderCacheInterfacePtr->cancelOrdersForSecIdWithMinimumQty(secId, 1000);
    orders = m_orderCacheInterfacePtr->getAllOrders();
    EXPECT_EQ(orders.size(), 0);
}

TEST_F(OrderCacheInterfaceTests, CancelOrdersForSecIdWithMinimumQty_TwoOrdersDifferentSecIdsOnList_Succeeds)
{
    static const std::string secId { "SecId1" };
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId0", "Buy", 1000, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", secId, "Buy", 1000, "User1", "CompanyA" });
    auto orders { m_orderCacheInterfacePtr->getAllOrders() };
    EXPECT_EQ(orders.size(), 2);
    m_orderCacheInterfacePtr->cancelOrdersForSecIdWithMinimumQty(secId, 1000);
    orders = m_orderCacheInterfacePtr->getAllOrders();
    EXPECT_EQ(orders.size(), 1);
    EXPECT_TRUE(orders[0].securityId() != secId);
}

TEST_F(OrderCacheInterfaceTests, CancelOrdersForSecIdWithMinimumQty_MultipleOrdersList_Succeeds)
{
    static const std::string secId { "SecId1" };
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", secId, "Buy", 1000, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", secId, "Buy", 3000, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", secId, "Buy", 100, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", secId, "Buy", 500, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId2", "Buy", 3000, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId3", "Buy", 2000, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId4", "Buy", 4000, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId5", "Buy", 5000, "User1", "CompanyA" });
    auto orders { m_orderCacheInterfacePtr->getAllOrders() };
    EXPECT_EQ(orders.size(), 8);
    m_orderCacheInterfacePtr->cancelOrdersForSecIdWithMinimumQty(secId, 1000);
    orders = m_orderCacheInterfacePtr->getAllOrders();
    EXPECT_EQ(orders.size(), 6);
}

TEST_F(OrderCacheInterfaceTests, GetMatchingSizeForSecurity_OneBuyOneSellOrdersSingleSecurity_Success)
{
    static const std::string secId { "SecId1" };
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", secId, "Sell", 1000, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId2", secId, "Buy", 1000, "User1", "CompanyB" });
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity(secId), 1000);
}

TEST_F(OrderCacheInterfaceTests, GetMatchingSizeForSecurity_ExampleFromReadMe_Success)
{
    // OrdId1 SecId1 Buy  1000 User1 CompanyA
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA" });
    // OrdId2 SecId2 Sell 3000 User2 CompanyB
    m_orderCacheInterfacePtr->addOrder({ "OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB" });
    // OrdId3 SecId1 Sell  500 User3 CompanyA
    m_orderCacheInterfacePtr->addOrder({ "OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA" });
    // OrdId4 SecId2 Buy   600 User4 CompanyC
    m_orderCacheInterfacePtr->addOrder({ "OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC" });
    // OrdId5 SecId2 Buy   100 User5 CompanyB
    m_orderCacheInterfacePtr->addOrder({ "OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB" });
    // OrdId6 SecId3 Buy  1000 User6 CompanyD
    m_orderCacheInterfacePtr->addOrder({ "OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD" });
    // OrdId7 SecId2 Buy  2000 User7 CompanyE
    m_orderCacheInterfacePtr->addOrder({ "OrdId7", "SecId2", "Buy", 2000, "User7", "CompanyE" });
    // OrdId8 SecId2 Sell 5000 User8 CompanyE
    m_orderCacheInterfacePtr->addOrder({ "OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE" });

    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity("SecId1"), 0);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity("SecId2"), 2700);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity("SecId3"), 0);
}

TEST_F(OrderCacheInterfaceTests, GetMatchingSizeForSecurity_Example1FromReadMe_Success)
{
    // OrdId1 SecId1 Sell 100 User10 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId1", "Sell", 100, "User10", "Company2" });
    // OrdId2 SecId3 Sell 200 User8 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId2", "SecId3", "Sell", 200, "User8", "Company2" });
    // OrdId3 SecId1 Buy 300 User13 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId3", "SecId1", "Buy", 300, "User13", "Company2" });
    // OrdId4 SecId2 Sell 400 User12 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId4", "SecId2", "Sell", 400, "User12", "Company2" });
    // OrdId5 SecId3 Sell 500 User7 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId5", "SecId3", "Sell", 500, "User7", "Company2" });
    // OrdId6 SecId3 Buy 600 User3 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId6", "SecId3", "Buy", 600, "User3", "Company1" });
    // OrdId7 SecId1 Sell 700 User10 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId7", "SecId1", "Sell", 700, "User10", "Company2" });
    // OrdId8 SecId1 Sell 800 User2 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId8", "SecId1", "Sell", 800, "User2", "Company1" });
    // OrdId9 SecId2 Buy 900 User6 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId9", "SecId2", "Buy", 900, "User6", "Company2" });
    // OrdId10 SecId2 Sell 1000 User5 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId10", "SecId2", "Sell", 1000, "User5", "Company1" });
    // OrdId11 SecId1 Sell 1100 User13 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId11", "SecId1", "Sell", 1100, "User13", "Company2" });
    // OrdId12 SecId2 Buy 1200 User9 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId12", "SecId2", "Buy", 1200, "User9", "Company2" });
    // OrdId13 SecId1 Sell 1300 User1 Company
    m_orderCacheInterfacePtr->addOrder({ "OrdId13", "SecId1", "Sell", 1300, "User1", "Company" });

    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity("SecId1"), 300);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity("SecId2"), 1000);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity("SecId3"), 600);
}

TEST_F(OrderCacheInterfaceTests, GetMatchingSizeForSecurity_Example2FromReadMe_Success)
{

    //                                 OrdId1 SecId3 Sell 100 User1 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId3", "Sell", 100, "User1", "Company1" });
    //                                 OrdId2 SecId3 Sell 200 User3 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId2", "SecId3", "Sell", 200, "User3", "Company2" });
    //                                 OrdId3 SecId1 Buy 300 User2 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId3", "SecId1", "Buy", 300, "User2", "Company1" });
    //                                 OrdId4 SecId3 Sell 400 User5 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId4", "SecId3", "Sell", 400, "User5", "Company2" });
    //                                 OrdId5 SecId2 Sell 500 User2 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId5", "SecId2", "Sell", 500, "User5", "Company1" });
    //                                 OrdId6 SecId2 Buy 600 User3 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId6", "SecId2", "Buy", 600, "User3", "Company2" });
    //                                 OrdId7 SecId2 Sell 700 User1 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId7", "SecId2", "Sell", 700, "User1", "Company1" });
    //                                 OrdId8 SecId1 Sell 800 User2 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId8", "SecId1", "Sell", 800, "User2", "Company1" });
    //                                 OrdId9 SecId1 Buy 900 User5 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId9", "SecId1", "Buy", 900, "User5", "Company2" });
    //                                 OrdId10 SecId1 Sell 1000 User1 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId10", "SecId1", "Sell", 1000, "User1", "Company1" });
    //                                 OrdId11 SecId2 Sell 1100 User6 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId11", "SecId2", "Sell", 1100, "User6", "Company2" });

    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity("SecId1"), 900);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity("SecId2"), 600);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity("SecId3"), 0);
}

//
TEST_F(OrderCacheInterfaceTests, GetMatchingSizeForSecurity2_OneBuyOneSellOrdersSingleSecurity_Success)
{
    static const std::string secId { "SecId1" };
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", secId, "Sell", 1000, "User1", "CompanyA" });
    m_orderCacheInterfacePtr->addOrder({ "OrdId2", secId, "Buy", 1000, "User1", "CompanyB" });
    EXPECT_EQ(m_orderCacheInterfacePtr->getAllOrders().size(), 2);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity2(secId), 1000);
    EXPECT_EQ(m_orderCacheInterfacePtr->getAllOrders().size(), 0);
}

TEST_F(OrderCacheInterfaceTests, GetMatchingSizeForSecurity2_ExampleFromReadMe_Success)
{
    // OrdId1 SecId1 Buy  1000 User1 CompanyA
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId1", "Buy", 1000, "User1", "CompanyA" });
    // OrdId2 SecId2 Sell 3000 User2 CompanyB
    m_orderCacheInterfacePtr->addOrder({ "OrdId2", "SecId2", "Sell", 3000, "User2", "CompanyB" });
    // OrdId3 SecId1 Sell  500 User3 CompanyA
    m_orderCacheInterfacePtr->addOrder({ "OrdId3", "SecId1", "Sell", 500, "User3", "CompanyA" });
    // OrdId4 SecId2 Buy   600 User4 CompanyC
    m_orderCacheInterfacePtr->addOrder({ "OrdId4", "SecId2", "Buy", 600, "User4", "CompanyC" });
    // OrdId5 SecId2 Buy   100 User5 CompanyB
    m_orderCacheInterfacePtr->addOrder({ "OrdId5", "SecId2", "Buy", 100, "User5", "CompanyB" });
    // OrdId6 SecId3 Buy  1000 User6 CompanyD
    m_orderCacheInterfacePtr->addOrder({ "OrdId6", "SecId3", "Buy", 1000, "User6", "CompanyD" });
    // OrdId7 SecId2 Buy  2000 User7 CompanyE
    m_orderCacheInterfacePtr->addOrder({ "OrdId7", "SecId2", "Buy", 2000, "User7", "CompanyE" });
    // OrdId8 SecId2 Sell 5000 User8 CompanyE
    m_orderCacheInterfacePtr->addOrder({ "OrdId8", "SecId2", "Sell", 5000, "User8", "CompanyE" });

    EXPECT_EQ(m_orderCacheInterfacePtr->getAllOrders().size(), 8);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity2("SecId1"), 0);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity2("SecId2"), 2700);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity2("SecId3"), 0);
    EXPECT_EQ(m_orderCacheInterfacePtr->getAllOrders().size(), 5);
}

TEST_F(OrderCacheInterfaceTests, GetMatchingSizeForSecurity2_Example1FromReadMe_Success)
{
    // OrdId1 SecId1 Sell 100 User10 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId1", "Sell", 100, "User10", "Company2" });
    // OrdId2 SecId3 Sell 200 User8 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId2", "SecId3", "Sell", 200, "User8", "Company2" });
    // OrdId3 SecId1 Buy 300 User13 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId3", "SecId1", "Buy", 300, "User13", "Company2" });
    // OrdId4 SecId2 Sell 400 User12 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId4", "SecId2", "Sell", 400, "User12", "Company2" });
    // OrdId5 SecId3 Sell 500 User7 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId5", "SecId3", "Sell", 500, "User7", "Company2" });
    // OrdId6 SecId3 Buy 600 User3 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId6", "SecId3", "Buy", 600, "User3", "Company1" });
    // OrdId7 SecId1 Sell 700 User10 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId7", "SecId1", "Sell", 700, "User10", "Company2" });
    // OrdId8 SecId1 Sell 800 User2 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId8", "SecId1", "Sell", 800, "User2", "Company1" });
    // OrdId9 SecId2 Buy 900 User6 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId9", "SecId2", "Buy", 900, "User6", "Company2" });
    // OrdId10 SecId2 Sell 1000 User5 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId10", "SecId2", "Sell", 1000, "User5", "Company1" });
    // OrdId11 SecId1 Sell 1100 User13 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId11", "SecId1", "Sell", 1100, "User13", "Company2" });
    // OrdId12 SecId2 Buy 1200 User9 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId12", "SecId2", "Buy", 1200, "User9", "Company2" });
    // OrdId13 SecId1 Sell 1300 User1 Company
    m_orderCacheInterfacePtr->addOrder({ "OrdId13", "SecId1", "Sell", 1300, "User1", "Company" });

    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity2("SecId1"), 300);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity2("SecId2"), 1000);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity2("SecId3"), 600);
}

TEST_F(OrderCacheInterfaceTests, GetMatchingSizeForSecurity2_Example2FromReadMe_Success)
{

    //                                 OrdId1 SecId3 Sell 100 User1 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId1", "SecId3", "Sell", 100, "User1", "Company1" });
    //                                 OrdId2 SecId3 Sell 200 User3 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId2", "SecId3", "Sell", 200, "User3", "Company2" });
    //                                 OrdId3 SecId1 Buy 300 User2 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId3", "SecId1", "Buy", 300, "User2", "Company1" });
    //                                 OrdId4 SecId3 Sell 400 User5 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId4", "SecId3", "Sell", 400, "User5", "Company2" });
    //                                 OrdId5 SecId2 Sell 500 User2 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId5", "SecId2", "Sell", 500, "User5", "Company1" });
    //                                 OrdId6 SecId2 Buy 600 User3 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId6", "SecId2", "Buy", 600, "User3", "Company2" });
    //                                 OrdId7 SecId2 Sell 700 User1 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId7", "SecId2", "Sell", 700, "User1", "Company1" });
    //                                 OrdId8 SecId1 Sell 800 User2 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId8", "SecId1", "Sell", 800, "User2", "Company1" });
    //                                 OrdId9 SecId1 Buy 900 User5 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId9", "SecId1", "Buy", 900, "User5", "Company2" });
    //                                 OrdId10 SecId1 Sell 1000 User1 Company1
    m_orderCacheInterfacePtr->addOrder({ "OrdId10", "SecId1", "Sell", 1000, "User1", "Company1" });
    //                                 OrdId11 SecId2 Sell 1100 User6 Company2
    m_orderCacheInterfacePtr->addOrder({ "OrdId11", "SecId2", "Sell", 1100, "User6", "Company2" });

    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity2("SecId1"), 900);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity2("SecId2"), 600);
    EXPECT_EQ(m_orderCacheInterfacePtr->getMatchingSizeForSecurity2("SecId3"), 0);
}
