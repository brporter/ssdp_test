#include <QtCore/QCoreApplication>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QNetworkDatagram>
#include <QtNetwork/QNetworkInterface>
#include <QTimer>
#include <vector>
#include <iostream>
#include <memory>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    std::vector<std::shared_ptr<QUdpSocket>> sockets;

    QHostAddress mcastGroup("239.255.255.250");
    QByteArray search("M-SEARCH * HTTP/1.1\r\nHost: 239.255.255.250:1900\r\nMan: \"ssdp:discover\"\r\nST: urn:schemas-upnp-org:device:BobLamp:1\r\n");

    for (auto ip : QNetworkInterface::allAddresses())
    {
        std::shared_ptr<QUdpSocket> sock = std::make_shared<QUdpSocket>();

        QObject::connect(sock.get(), &QUdpSocket::readyRead, &a, [&]() {
            while (sock->hasPendingDatagrams())
            {
                QNetworkDatagram dgram = sock->receiveDatagram();

                std::cout << "Got response of " << dgram.data().size() << " from " << dgram.senderAddress().toString().toStdString() << "\n" << std::endl;
                std::cout << "Payload was: \n" << dgram.data().data() << "\n";
                std::cout << "-----" << std::endl;
            }
        });

        sock->bind(ip, 0, QUdpSocket::ShareAddress);

        sockets.push_back(sock);
        std::cout << "Socket bound to " << sock->localAddress().toString().toStdString() << ":" << sock->localPort() << std::endl;
    }

    QTimer poll { nullptr };
    QObject::connect(&poll, &QTimer::timeout, &a, [&]() {
        for (auto sock : sockets)
        {
            std::cout << "Polling on socket bound to " << sock->localAddress().toString().toStdString() << ":" << sock->localPort() << std::endl;
            sock->writeDatagram(search, mcastGroup, 1900);
        }
    });

    poll.start(1000);

    return a.exec();
}
