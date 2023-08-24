#pragma once

#include <QObject>
#include "3rd_party/oxygine-framework/oxygine/actor/Actor.h"

class BaseTableItem;
using spBaseTableItem = std::shared_ptr<BaseTableItem>;

class BaseTableItem : public QObject, public oxygine::Actor
{
    Q_OBJECT
public:
    explicit BaseTableItem(QObject *parent = nullptr);
    virtual ~BaseTableItem() = default;

    virtual bool operator>(const BaseTableItem& rhs) const = 0;
    inline bool operator<(const BaseTableItem& rhs) const
    {
        return rhs > *this;
    }
};

