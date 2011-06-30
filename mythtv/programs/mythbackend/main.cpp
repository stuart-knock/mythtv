#ifndef _WIN32
#include <QCoreApplication>
#else
#include <QApplication>
#endif

#include <QFileInfo>
#include <QRegExp>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QMap>

#include "commandlineparser.h"
#include "scheduledrecording.h"
#include "previewgenerator.h"
#include "mythcorecontext.h"
#include "mythsystemevent.h"
#include "backendcontext.h"
#include "main_helpers.h"
#include "storagegroup.h"
#include "housekeeper.h"
#include "mediaserver.h"
#include "mythlogging.h"
#include "mythversion.h"
#include "programinfo.h"
#include "autoexpire.h"
#include "mainserver.h"
#include "remoteutil.h"
#include "exitcodes.h"
#include "scheduler.h"
#include "jobqueue.h"
#include "dbcheck.h"
#include "compat.h"
#include "mythdb.h"
#include "tv_rec.h"


#define LOC      QString("MythBackend: ")
#define LOC_WARN QString("MythBackend, Warning: ")
#define LOC_ERR  QString("MythBackend, Error: ")

#ifdef Q_OS_MACX
    // 10.6 uses some file handles for its new Grand Central Dispatch thingy
    #define UNUSED_FILENO 5
#else
    #define UNUSED_FILENO 3
#endif

static void qt_exit(int)
{
    signal(SIGINT, SIG_DFL);
    QCoreApplication::exit(0);
}

int main(int argc, char **argv)
{
    MythBackendCommandLineParser cmdline;
    if (!cmdline.Parse(argc, argv))
    {
        cmdline.PrintHelp();
        return GENERIC_EXIT_INVALID_CMDLINE;
    }

    if (cmdline.toBool("showhelp"))
    {
        cmdline.PrintHelp();
        return GENERIC_EXIT_OK;
    }

    if (cmdline.toBool("showversion"))
    {
        cmdline.PrintVersion();
        return GENERIC_EXIT_OK;
    }

#ifndef _WIN32
    for (int i = UNUSED_FILENO; i < sysconf(_SC_OPEN_MAX) - 1; ++i)
        close(i);
    QCoreApplication a(argc, argv);
#else
    // MINGW application needs a window to receive messages
    // such as socket notifications :[
    QApplication a(argc, argv);
#endif
    QCoreApplication::setApplicationName(MYTH_APPNAME_MYTHBACKEND);

    pidfile = cmdline.toString("pidfile");

    bool daemonize = cmdline.toBool("daemon");
    int retval;
    QString mask("important general");
    if ((retval = cmdline.ConfigureLogging(mask, daemonize)) != GENERIC_EXIT_OK)
        return retval;

    ///////////////////////////////////////////////////////////////////////
    // Don't listen to console input
    close(0);

    CleanupGuard callCleanup(cleanup);
    signal(SIGINT, qt_exit);
    signal(SIGTERM, qt_exit);

    int exitCode = setup_basics(cmdline);
    if (exitCode != GENERIC_EXIT_OK)
        return exitCode;

    setHttpProxy();

    gContext = new MythContext(MYTH_BINARY_VERSION);

    if (cmdline.toBool("event")         || cmdline.toBool("systemevent") ||
        cmdline.toBool("setverbose")    || cmdline.toBool("printsched") ||
        cmdline.toBool("testsched")     || cmdline.toBool("resched") ||
        cmdline.toBool("scanvideos")    || cmdline.toBool("clearcache") ||
        cmdline.toBool("printexpire"))
    {
        if (!setup_context(cmdline))
            return GENERIC_EXIT_NO_MYTHCONTEXT;
        return handle_command(cmdline);
    }

    /////////////////////////////////////////////////////////////////////////
    // Not sure we want to keep running the backend when there is an error.
    // Currently, it keeps repeating the same error over and over.
    // Removing loop until reason for having it is understood.
    //
    //while (true)
    //{
        exitCode = run_backend(cmdline);
    //}

    return exitCode;
}

/* vim: set expandtab tabstop=4 shiftwidth=4: */
