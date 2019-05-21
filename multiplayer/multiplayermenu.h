#ifndef MULTIPLAYERMENU_H
#define MULTIPLAYERMENU_H

#include "memory.h"

#include <QObject>
#include <QTimer>
#include "oxygine-framework.h"

#include "objects/chat.h"

#include "menue/mapselectionmapsmenue.h"
#include "network/NetworkInterface.h"

class Multiplayermenu : public MapSelectionMapsMenue
{
    Q_OBJECT
public:
    explicit Multiplayermenu(QString adress, bool host);
    virtual ~Multiplayermenu() = default;

    /**
     * @brief existsMap
     * @param fileName
     * @param hash
     */
    bool existsMap(QString& fileName, QByteArray& hash);
    /**
     * @brief createChat
     */
    void createChat();
    /**
     * @brief disconnectNetwork
     */
    void disconnectNetwork();
signals:

public slots:

    // general slots
    virtual void slotButtonBack() override;
    virtual void slotButtonNext() override;
    virtual void startGame() override;

    // network slots
    void playerJoined(quint64 socketID);
    void disconnected(quint64 socketID);
    void recieveData(quint64 socketID, QByteArray data, NetworkInterface::NetworkSerives service);
protected slots:
    void countdown();
protected:
    bool getGameReady();
    void sendServerReady(bool value);
private:
    bool m_Host{false};
    spNetworkInterface m_NetworkInterface;
    oxygine::spTextField m_pHostAdresse;
    spChat m_Chat;
    QTimer m_GameStartTimer;
    qint32 counter{5};
};

#endif // MULTIPLAYERMENU_H
