#ifndef SCTCPSERVERWIDGET_H
#define SCTCPSERVERWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class SCTcpServerWidget; }
QT_END_NAMESPACE

class SCTcpServerThread;
class SCTcpServerWidget : public QWidget
{
    Q_OBJECT

public:
    SCTcpServerWidget(QWidget *parent = nullptr);
    ~SCTcpServerWidget();

private slots:
    void onConnectionSucceed(QString strHostAddress, int nPort);
    void onDataReaded(QString data);
    void onSocketNumChanged(int nNum);

private:
    Ui::SCTcpServerWidget *ui;
    QVector<SCTcpServerThread*> m_vecServerThread;   // 控制服务器的线程
};
#endif // SCTCPSERVERWIDGET_H
