#include "TvShowModelItem.h"

#include <QApplication>
#include <QStringList>
#include "Globals.h"

TvShowModelItem::TvShowModelItem(TvShowModelItem *parent) :
    QObject(0)
{
    moveToThread(QApplication::instance()->thread());
    m_parentItem = parent;
    m_tvShow = 0;
    m_tvShowEpisode = 0;
}

TvShowModelItem::~TvShowModelItem()
{
    qDeleteAll(m_childItems);
}

TvShowModelItem *TvShowModelItem::child(int number)
{
    return m_childItems.value(number);
}

int TvShowModelItem::childCount() const
{
    return m_childItems.count();
}

int TvShowModelItem::childNumber() const
{
    if (m_parentItem)
        return m_parentItem->m_childItems.indexOf(const_cast<TvShowModelItem*>(this));

    return 0;
}

int TvShowModelItem::columnCount() const
{
    return 1;
}

QVariant TvShowModelItem::data(int column) const
{
    switch (column)
    {
    case 3:
        if (m_tvShow)
            return m_tvShow->hasNewEpisodes();
        else if (m_tvShowEpisode)
            return !m_tvShowEpisode->infoLoaded();
        break;
    case 2:
        if (m_tvShow)
            return m_tvShow->hasChanged();
        else if (m_tvShowEpisode)
            return m_tvShowEpisode->hasChanged();
        break;
    case 1:
        if (m_tvShow)
            return m_tvShow->episodeCount();
        break;
    default:
        if (m_tvShow)
            return m_tvShow->name();
        else if (m_tvShowEpisode)
            return m_tvShowEpisode->completeEpisodeName();
        else if (!m_season.isEmpty())
            return tr("Season %1").arg(m_season);
        break;
    }
    return QVariant();
}

TvShowModelItem *TvShowModelItem::appendChild(TvShow *show)
{
    TvShowModelItem *item = new TvShowModelItem(this);
    item->setTvShow(show);
    show->setModelItem(item);
    m_childItems.append(item);
    return item;
}

TvShowModelItem *TvShowModelItem::appendChild(TvShowEpisode *episode)
{
    TvShowModelItem *item = new TvShowModelItem(this);
    item->setTvShowEpisode(episode);
    episode->setModelItem(item);
    m_childItems.append(item);
    connect(episode, SIGNAL(sigChanged(TvShowEpisode*)), this, SLOT(onTvShowEpisodeChanged(TvShowEpisode*)), Qt::UniqueConnection);
    return item;
}

TvShowModelItem *TvShowModelItem::appendChild(QString season)
{
    TvShowModelItem *item = new TvShowModelItem(this);
    item->setSeason(season);
    m_childItems.append(item);
    connect(item, SIGNAL(sigIntChanged(TvShowModelItem*,TvShowModelItem*)), this, SLOT(onSeasonChanged(TvShowModelItem*, TvShowModelItem*)), Qt::UniqueConnection);
    return item;
}

TvShowModelItem *TvShowModelItem::parent()
{
    return m_parentItem;
}

bool TvShowModelItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > m_childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete m_childItems.takeAt(position);

    return true;
}

void TvShowModelItem::setTvShow(TvShow *show)
{
    m_tvShow = show;
}

void TvShowModelItem::setTvShowEpisode(TvShowEpisode *episode)
{
    m_tvShowEpisode = episode;
}

void TvShowModelItem::setSeason(QString season)
{
    m_season = season;
}

TvShow *TvShowModelItem::tvShow()
{
    return m_tvShow;
}

TvShowEpisode *TvShowModelItem::tvShowEpisode()
{
    return m_tvShowEpisode;
}

QString TvShowModelItem::season()
{
    return m_season;
}

int TvShowModelItem::type()
{
    if (m_tvShow)
        return TypeTvShow;
    else if (m_tvShowEpisode)
        return TypeEpisode;
    else if (!m_season.isEmpty())
        return TypeSeason;

    return -1;
}

void TvShowModelItem::onTvShowEpisodeChanged(TvShowEpisode *episode)
{
    emit sigIntChanged(this, episode->modelItem());
}

void TvShowModelItem::onSeasonChanged(TvShowModelItem *seasonItem, TvShowModelItem *episodeItem)
{
    emit sigChanged(this, seasonItem, episodeItem);
}
