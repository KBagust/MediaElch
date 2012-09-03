#include "MovieFileSearcher.h"

#include <QApplication>
#include <QDebug>
#include "Manager.h"

MovieFileSearcher::MovieFileSearcher(QObject *parent) :
    QThread(parent)
{
    m_progressMessageId = Constants::MovieFileSearcherProgressMessageId;
}

MovieFileSearcher::~MovieFileSearcher()
{
}

void MovieFileSearcher::run()
{
    emit searchStarted(tr("Searching for Movies..."), m_progressMessageId);

    Manager::instance()->movieModel()->clear();
    QList<QStringList> contents;
    foreach (SettingsDir dir, m_directories) {
        getDirContents(dir.path, contents);
    }

    int i=0;
    int n=contents.size();
    foreach (const QStringList &files, contents) {
        bool inSeparateFolder = false;
        // get directory
        if (!files.isEmpty()) {
            int index = -1;
            for (int i=0, n=m_directories.count() ; i<n ; ++i) {
                if (files.at(0).startsWith(m_directories[i].path)) {
                    if (index == -1)
                        index = i;
                    else if (m_directories[index].path.length() < m_directories[i].path.length())
                        index = i;
                }
            }
            if (index != -1)
                inSeparateFolder = m_directories[index].separateFolders;
        }
        Movie *movie = new Movie(files);
        movie->setInSeparateFolder(inSeparateFolder);
        movie->loadData(Manager::instance()->mediaCenterInterface());
        Manager::instance()->movieModel()->addMovie(movie);
        emit progress(++i, n, m_progressMessageId);
    }

    emit moviesLoaded(m_progressMessageId);
}

void MovieFileSearcher::setMovieDirectories(QList<SettingsDir> directories)
{
    m_directories.clear();
    for (int i=0, n=directories.count() ; i<n ; ++i) {
        QFileInfo fi(directories.at(i).path);
        if (fi.isDir())
            m_directories.append(directories.at(i));
    }
}

void MovieFileSearcher::getDirContents(QString path, QList<QStringList> &contents)
{
    QDir dir(path);
    foreach (const QString &cDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        // skip bluray backup folder
        if ((QString::compare(cDir, "BACKUP", Qt::CaseInsensitive) == 0 && dir.path().endsWith("BDMV", Qt::CaseInsensitive)) ||
			(QString::compare(cDir, "Extras", Qt::CaseInsensitive) == 0))
			continue;
        this->getDirContents(path + QDir::separator() + cDir, contents);
    }

    QStringList filters;
    QStringList files;
    filters << "*.mkv" << "*.avi" << "*.mpg" << "*.mpeg" << "*.mp4" << "*.iso" << "*.m2ts" << "VIDEO_TS.IFO" << "index.bdmv" << "*.disc" << "*.m4v";
    foreach (const QString &file, dir.entryList(filters, QDir::Files | QDir::System)) {
        if (file.endsWith(".m2ts", Qt::CaseInsensitive) && (dir.path().endsWith("BDMV/STREAM", Qt::CaseInsensitive) ||
                                                             dir.path().endsWith("BDMV\\STREAM", Qt::CaseInsensitive)))
            continue;
        if (file.contains("-trailer", Qt::CaseInsensitive))
            continue;
        files.append(file);
    }
    files.sort();

    QRegExp rx("((part|cd)[\\s_]*)(\\d+)", Qt::CaseInsensitive);
    for (int i=0, n=files.size() ; i<n ; i++) {
        QStringList movieFiles;
        QString file = files.at(i);
        if (file.isEmpty())
            continue;

        movieFiles << path + QDir::separator() + file;

        if (QString::compare(file, "VIDEO_TS.IFO", Qt::CaseInsensitive) != 0 && QString::compare(file, "index.bdmv", Qt::CaseInsensitive) != 0) {
            int pos = rx.indexIn(file);
            if (pos != -1) {
                QString left = file.left(pos) + rx.cap(1);
                QString right = file.mid(pos+rx.cap(1).size()+rx.cap(2).size());
                for (int x=0 ; x<n ; x++) {
                    QString subFile = files.at(x);
                    if (subFile != file) {
                        if (subFile.startsWith(left) && subFile.endsWith(right)) {
                            movieFiles << path + QDir::separator() + subFile;
                            files[x] = ""; // set an empty file name, this way we can skip this file in the main loop
                        }
                    }
                }
            }
        }

        contents.append(movieFiles);
    }
}
