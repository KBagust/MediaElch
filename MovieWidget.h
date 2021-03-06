#ifndef MOVIEWIDGET_H
#define MOVIEWIDGET_H

#include <QContextMenuEvent>
#include <QLabel>
#include <QMenu>
#include <QResizeEvent>
#include <QTableWidgetItem>
#include <QWidget>

#include "data/Movie.h"
#include "DownloadManager.h"

namespace Ui {
class MovieWidget;
}

class MovieWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MovieWidget(QWidget *parent = 0);
    ~MovieWidget();

public slots:
    void clear();
    void setMovie(Movie *movie);
    void startScraperSearch();
    void saveInformation();
    void saveAll();
    void setEnabledTrue(Movie *movie = 0);
    void setDisabledTrue();

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    void actorDownloadStarted(QString, int);
    void actorDownloadProgress(int, int, int);
    void actorDownloadFinished(int);
    void setActionSearchEnabled(bool, MainWidgets);
    void setActionSaveEnabled(bool, MainWidgets);

private slots:
    void downloadActorsFinished(Movie *movie);
    void actorDownloadsLeft(int left, DownloadManagerElement elem);
    void loadDone(Movie *movie);
    void chooseMoviePoster();
    void chooseMovieBackdrop();
    void posterDownloadFinished(DownloadManagerElement elem);
    void movieNameChanged(QString text);
    void addGenre();
    void removeGenre();
    void addActor();
    void removeActor();
    void addStudio();
    void removeStudio();
    void addCountry();
    void removeCountry();
    void onPreviewPoster();
    void onPreviewBackdrop();
    void onActorChanged();
    void onChangeActorImage();
    void onRevertChanges();

    void onNameChange(QString text);
    void onOriginalNameChange(QString text);
    void onSetChange(QString text);
    void onTaglineChange(QString text);
    void onRatingChange(double value);
    void onReleasedChange(QDate date);
    void onRuntimeChange(int value);
    void onCertificationChange(QString text);
    void onTrailerChange(QString text);
    void onWatchedChange(int state);
    void onPlayCountChange(int value);
    void onLastWatchedChange(QDateTime dateTime);
    void onOverviewChange();

    void onActorEdited(QTableWidgetItem *item);
    void onCountryEdited(QTableWidgetItem *item);
    void onGenreEdited(QTableWidgetItem *item);
    void onStudioEdited(QTableWidgetItem *item);

private:
    Ui::MovieWidget *ui;
    Movie *m_movie;
    DownloadManager *m_posterDownloadManager;
    QMovie *m_loadingMovie;
    QLabel *m_savingWidget;
    QImage m_currentPoster;
    QImage m_currentBackdrop;
    void updateMovieInfo();
};

#endif // MOVIEWIDGET_H
