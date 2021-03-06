#ifndef MEDIACENTERINTERFACE_H
#define MEDIACENTERINTERFACE_H

#include "Globals.h"
#include "data/Movie.h"
#include "data/TvShow.h"
#include "data/TvShowEpisode.h"

class Movie;
class TvShow;
class TvShowEpisode;

class MediaCenterInterface : public QObject
{
public:
    virtual bool saveMovie(Movie *movie) = 0;
    virtual bool loadMovie(Movie *movie) = 0;
    virtual void loadMovieImages(Movie *movie) = 0;
    virtual void exportDatabase(QList<Movie*> movies, QList<TvShow*> shows, QString exportPath, QString pathSearch, QString pathReplace) = 0;
    virtual bool loadTvShow(TvShow *show) = 0;
    virtual void loadTvShowImages(TvShow *show) = 0;
    virtual bool loadTvShowEpisode(TvShowEpisode *episode) = 0;
    virtual void loadTvShowEpisodeImages(TvShowEpisode *episode) = 0;
    virtual QImage movieSetPoster(QString setName) = 0;
    virtual QImage movieSetBackdrop(QString setName) = 0;
    virtual void saveMovieSetPoster(QString setName, QImage poster) = 0;
    virtual void saveMovieSetBackdrop(QString setName, QImage backdrop) = 0;
    virtual bool saveTvShow(TvShow *show) = 0;
    virtual bool saveTvShowEpisode(TvShowEpisode *episode) = 0;
    virtual void shutdown() = 0;
    virtual bool hasFeature(int feature) = 0;
signals:
    virtual void sigExportStarted() = 0;
    virtual void sigExportProgress(int, int) = 0;
    virtual void sigExportDone() = 0;
    virtual void sigExportRaiseError(QString) = 0;
};

#endif // MEDIACENTERINTERFACE_H
