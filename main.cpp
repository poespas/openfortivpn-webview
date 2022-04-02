#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QCursor>
#include <QDesktopWidget>
#include <QRect>
#include <QScreen>

static QScreen *findScreenWithCursor()
{
    QPoint pos = QCursor::pos();

    for (QScreen *screen : QGuiApplication::screens()) {
        QRect screenRect = screen->geometry();
        if (screenRect.contains(pos)) {
            return screen;
        }
    }

    return QApplication::desktop()->screen();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto optionRealm = QCommandLineOption("realm", "The authentication realm.", "realm");
    auto optionUrl = QCommandLineOption("url", "The already built SAML URL.\nThis takes precedence over [host:port].", "url");
    auto optionKeepOpen = QCommandLineOption("keep-open", "Do not close the browser automatically.");

    QCommandLineParser parser;
    parser.addPositionalArgument("host", "The VPN gateway host with an optional port.", "[host:port]");
    parser.addOption(optionRealm);
    parser.addOption(optionUrl);
    parser.addOption(optionKeepOpen);
    parser.addOption(QCommandLineOption("remote-debugging-port", "Remote debugging server port."));
    parser.addHelpOption();
    parser.addVersionOption();

    if (!parser.parse(QCoreApplication::arguments())) {
        parser.showHelp(1);
    }

    if (parser.isSet("help")) {
        parser.showHelp(0);
    }

    QString url = parser.value(optionUrl);
    if (url.isEmpty()) {
        if (parser.positionalArguments().size() < 1) {
            parser.showHelp(1);
        }
        url = "https://" + parser.positionalArguments()[0] + "/remote/saml/start";
        QString realm = parser.value(optionRealm);
        if (!realm.isEmpty()) {
            url += "?realm=" + realm;
        }
    }

    bool keepOpen = parser.isSet(optionKeepOpen);

    MainWindow w(keepOpen);

    w.loadUrl(url);
    w.resize(1024, 760);
    w.move(findScreenWithCursor()->geometry().center() - w.rect().center());
    w.show();

    return app.exec();
}
