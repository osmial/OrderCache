#include "order.hpp"

unsigned int Order::operator+=(unsigned int qtyChange)
{
    m_qty += qtyChange;
    return m_qty;
}

unsigned int Order::operator-=(unsigned int qtyChange)
{
    m_qty -= qtyChange;
    return m_qty;
}
