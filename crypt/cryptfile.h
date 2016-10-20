#ifndef CRYPTFILE_H
#define CRYPTFILE_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QString>
#include <QMouseEvent>
#include <QThread>
#include <QProgressBar>



class OperaThread : public QThread
{
    Q_OBJECT
public:
   OperaThread(QStringList cfileRounts, QString cpassword, QString cnewFileName, QString cnewSuffix);
private:
   QStringList fileRounts;
   QString password;
   QString newFileName;
   QString newSuffix;
   void run();
signals:
    void update_m(QString result, int point, int count);
};









class Customm : public QDialog
{
    Q_OBJECT

public:
    Customm(QWidget *parent = 0);
    ~Customm();

private:
    QLabel *lbFileName, *lbSuffix, *lbFileNameHint, *lbSuffixHint;
    QLineEdit *leFileName, *leSuffix;
    QPushButton *pbSure, *pbCancel;

signals:
    void parameters_m(QString filename, QString suffix);

private slots:
    void sure();
    void cancel();
};

class Explain : public QDialog
{
    Q_OBJECT

public:
    Explain(QWidget *parent = 0);
    ~Explain();

private:
    QLabel *lbImage, *lbCopyRight, *lbcontext;

};









class CryptFile : public QDialog
{
    Q_OBJECT

public:
    CryptFile(QWidget *parent = 0);
    ~CryptFile();
    recursionDir(QString path, QString prefix);

    virtual bool eventFilter(QObject *, QEvent *);

private:
    QLabel *lbFileHint, *lbImageDlam, *lbSummary, *lbFileSize, *lbCrypt;
    QLineEdit *leFileAddress, *leCrypt;
    QPushButton *pbFileOpen, *pbStart, *pbExplain, *pbSummary;
    QTextEdit *teDisplay;

    QStringList fileRounts;
    QString fileNames;
    QString newFileName;
    QString newSuffix;
    QString resultDisplay;

    OperaThread *otStart;
    Customm *setCustomm;
    Explain *setExplain;

    int fileNum;


    void runExplain();

private slots:
    void update_n(QString result, int point, int count);
    void parameters_n(QString filename, QString suffix);
    void setFileRoute();
    void startCrypt();
    void runCustomm();

};



#endif // CRYPTFILE_H
