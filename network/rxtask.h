#ifndef RXTASK_H
#define RXTASK_H

#include <QObject>
#include <QDataStream>

#include "network/networkInterface.h"

class QIODevice;
class RxTask;
using spRxTask = std::shared_ptr<RxTask>;

class RxTask final : public QObject
{
    Q_OBJECT
public:
    RxTask(QIODevice* pSocket, quint64 socketID, NetworkInterface* CommIF, bool useReceivedId);
    virtual ~RxTask() = default;
    void swapNetworkInterface(NetworkInterface* pCommIF)
    {
        m_pIF = pCommIF;
    }
    quint64 getSocketID() const;
    void setSocketID(const quint64 &SocketID);
    void close();
public slots:
    void recieveData();
private:
   QIODevice* m_pSocket;
   NetworkInterface* m_pIF;
   quint64 m_SocketID;
   QDataStream m_pStream;
   bool m_useReceivedId{false};
};

#endif // RXTASK_H
