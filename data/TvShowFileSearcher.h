#ifndef TVSHOWFILESEARCHER_H
#define TVSHOWFILESEARCHER_H

#include <QDir>
#include <QThread>
#include "Globals.h"

class TvShowFileSearcher : public QThread
{
    Q_OBJECT
public:
    explicit TvShowFileSearcher(QObject *parent = 0);
    void run();
    void setMovieDirectories(QList<SettingsDir> directories);

signals:
    void searchStarted(QString, int);
    void progress(int, int, int);
    void tvShowsLoaded(int);

private:
    QStringList m_directories;
    int m_progressMessageId;
    void getTvShows(QString path, QMap<QString, QList<QStringList> > &contents);
    void getSubDirs(QDir dir, QStringList &subDirs);
};

#endif // TVSHOWFILESEARCHER_H
