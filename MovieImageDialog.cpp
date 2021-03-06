#include "MovieImageDialog.h"
#include "ui_MovieImageDialog.h"

#include <QDebug>
#include <QFileDialog>
#include <QSettings>
#include <QtCore/qmath.h>
#include <QLabel>
#include <QMovie>
#include <QPainter>
#include <QSize>
#include <QTimer>

MovieImageDialog::MovieImageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MovieImageDialog)
{
    ui->setupUi(this);

#ifdef Q_WS_MAC
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Sheet);
    setStyleSheet(styleSheet() + " #MovieImageDialog { border: 1px solid rgba(0, 0, 0, 100); border-top: none; }");
#else
    setWindowFlags((windowFlags() & ~Qt::WindowType_Mask) | Qt::Dialog);
#endif

    QSettings settings;
    resize(settings.value("MovieImageDialog/Size").toSize());

    connect(ui->table, SIGNAL(cellClicked(int,int)), this, SLOT(imageClicked(int, int)));
    connect(ui->table, SIGNAL(sigDroppedImage(QUrl)), this, SLOT(onImageDropped(QUrl)));
    connect(ui->buttonClose, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->buttonChoose, SIGNAL(clicked()), this, SLOT(chooseLocalImage()));
    connect(ui->previewSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(onPreviewSizeChange(int)));
    connect(ui->buttonZoomIn, SIGNAL(clicked()), this, SLOT(onZoomIn()));
    connect(ui->buttonZoomOut, SIGNAL(clicked()), this, SLOT(onZoomOut()));
    QMovie *movie = new QMovie(":/img/spinner.gif");
    movie->start();
    ui->labelSpinner->setMovie(movie);
    this->clear();
    this->setImageType(TypePoster);
    m_currentDownloadReply = 0;

    QPixmap zoomOut(":/img/zoom_out.png");
    QPixmap zoomIn(":/img/zoom_in.png");
    QPainter p;
    p.begin(&zoomOut);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(zoomOut.rect(), QColor(0, 0, 0, 150));
    p.end();
    p.begin(&zoomIn);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(zoomIn.rect(), QColor(0, 0, 0, 150));
    p.end();
    ui->buttonZoomOut->setIcon(QIcon(zoomOut));
    ui->buttonZoomIn->setIcon(QIcon(zoomIn));

    m_noElementsLabel = new QLabel(tr("No images found"), ui->table);
    m_noElementsLabel->setMargin(10);
    m_noElementsLabel->hide();
}

MovieImageDialog::~MovieImageDialog()
{
    delete ui;
}

int MovieImageDialog::exec(int type)
{
    m_type = type;
    QSettings settings;
    ui->previewSizeSlider->setValue(settings.value(QString("MovieImageDialog/PreviewSize_%1").arg(m_type), 8).toInt());
    QSize newSize;
    newSize.setHeight(parentWidget()->size().height()-50);
    newSize.setWidth(qMin(1200, parentWidget()->size().width()-100));
    resize(newSize);

    int xMove = (parentWidget()->size().width()-size().width())/2;
    QPoint globalPos = parentWidget()->mapToGlobal(parentWidget()->pos());
    move(globalPos.x()+xMove, globalPos.y());

    return QDialog::exec();
}

void MovieImageDialog::accept()
{
    this->cancelDownloads();
    QSettings settings;
    settings.setValue("MovieImageDialog/Size", size());
    QDialog::accept();
}

void MovieImageDialog::reject()
{
    this->cancelDownloads();
    QSettings settings;
    settings.setValue("MovieImageDialog/Size", size());
    QDialog::reject();
}

MovieImageDialog *MovieImageDialog::instance(QWidget *parent)
{
    static MovieImageDialog *m_instance = 0;
    if (m_instance == 0) {
        m_instance = new MovieImageDialog(parent);
    }
    return m_instance;
}

void MovieImageDialog::clear()
{
    this->cancelDownloads();
    m_elements.clear();
    ui->table->clearContents();
    ui->table->setRowCount(0);
}

QUrl MovieImageDialog::imageUrl()
{
    return m_imageUrl;
}

void MovieImageDialog::resizeEvent(QResizeEvent *event)
{
    if (this->calcColumnCount() != ui->table->columnCount())
        this->renderTable();
    QWidget::resizeEvent(event);
}

void MovieImageDialog::setDownloads(QList<Poster> downloads)
{
    foreach (const Poster &poster, downloads) {
        DownloadElement d;
        d.originalUrl = poster.originalUrl;
        d.thumbUrl = poster.thumbUrl;
        d.downloaded = false;
        d.resolution = poster.originalSize;
        this->m_elements.append(d);
    }
    ui->labelLoading->setVisible(true);
    ui->labelSpinner->setVisible(true);
    this->startNextDownload();
    this->renderTable();
}

QNetworkAccessManager *MovieImageDialog::qnam()
{
    return &m_qnam;
}

void MovieImageDialog::startNextDownload()
{
    int nextIndex = -1;
    for (int i=0, n=m_elements.size() ; i<n ; i++) {
        if (!m_elements[i].downloaded) {
            nextIndex = i;
            break;
        }
    }

    if (nextIndex == -1) {
        ui->labelLoading->setVisible(false);
        ui->labelSpinner->setVisible(false);
        return;
    }

    m_currentDownloadIndex = nextIndex;
    m_currentDownloadReply = this->qnam()->get(QNetworkRequest(m_elements[nextIndex].thumbUrl));
    connect(m_currentDownloadReply, SIGNAL(finished()), this, SLOT(downloadFinished()));
}

void MovieImageDialog::downloadFinished()
{
    if (m_currentDownloadReply->error() != QNetworkReply::NoError) {
        this->startNextDownload();
        return;
    }

    m_elements[m_currentDownloadIndex].pixmap.loadFromData(m_currentDownloadReply->readAll());
    if (!m_elements[m_currentDownloadIndex].pixmap.isNull()) {
        m_elements[m_currentDownloadIndex].scaledPixmap = m_elements[m_currentDownloadIndex].pixmap.scaledToWidth(this->getColumnWidth()-10, Qt::SmoothTransformation);
        m_elements[m_currentDownloadIndex].cellWidget->setImage(m_elements[m_currentDownloadIndex].scaledPixmap);
        m_elements[m_currentDownloadIndex].cellWidget->setResolution(m_elements[m_currentDownloadIndex].resolution);
    }
    ui->table->resizeRowsToContents();
    m_elements[m_currentDownloadIndex].downloaded = true;
    m_currentDownloadReply->deleteLater();
    this->startNextDownload();
}

void MovieImageDialog::renderTable()
{
    int cols = this->calcColumnCount();
    ui->table->setColumnCount(cols);
    ui->table->setRowCount(0);
    ui->table->clearContents();

    for (int i=0, n=ui->table->columnCount() ; i<n ; i++)
        ui->table->setColumnWidth(i, this->getColumnWidth());

    for (int i=0, n=m_elements.size() ; i<n ; i++) {
        int row = (i-(i%cols))/cols;
        if (i%cols == 0)
            ui->table->insertRow(row);
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData(Qt::UserRole, m_elements[i].originalUrl);
        ImageLabel *label = new ImageLabel(ui->table);
        if (!m_elements[i].pixmap.isNull()) {
            label->setImage(m_elements[i].pixmap.scaledToWidth(this->getColumnWidth()-10, Qt::SmoothTransformation));
            label->setResolution(m_elements[i].resolution);
        }
        m_elements[i].cellWidget = label;
        ui->table->setItem(row, i%cols, item);
        ui->table->setCellWidget(row, i%cols, label);
        ui->table->resizeRowToContents(row);
    }

    m_noElementsLabel->setVisible(m_elements.size() == 0);
}

int MovieImageDialog::calcColumnCount()
{
    int width = ui->table->size().width();
    int colWidth = this->getColumnWidth()+4;
    int cols = qFloor((qreal)width/colWidth);
    return cols;
}

int MovieImageDialog::getColumnWidth()
{
    return ui->previewSizeSlider->value()*16;
}

void MovieImageDialog::imageClicked(int row, int col)
{
    if (ui->table->item(row, col) == 0)
        return;
    QUrl url = ui->table->item(row, col)->data(Qt::UserRole).toUrl();
    m_imageUrl = url;
    accept();
}

void MovieImageDialog::setImageType(ImageType type)
{
    m_imageType = type;
}

void MovieImageDialog::cancelDownloads()
{
    ui->labelLoading->setVisible(false);
    ui->labelSpinner->setVisible(false);
    bool running = false;
    foreach (const DownloadElement &d, m_elements) {
        if (!d.downloaded) {
            running = true;
            break;
        }
    }
    m_elements.clear();
    if (running)
        m_currentDownloadReply->abort();
}

void MovieImageDialog::chooseLocalImage()
{
    QString fileName = QFileDialog::getOpenFileName(parentWidget(), tr("Choose Image"), QDir::homePath(), tr("Images (*.jpg *.jpeg *.png)"));
    if (!fileName.isNull()) {
        int index = m_elements.size();
        DownloadElement d;
        d.originalUrl = fileName;
        d.thumbUrl = fileName;
        d.downloaded = false;
        m_elements.append(d);
        this->renderTable();
        m_elements[index].pixmap = QPixmap(fileName);
        m_elements[index].pixmap = m_elements[index].pixmap.scaledToWidth(this->getColumnWidth()-10, Qt::SmoothTransformation);
        m_elements[index].cellWidget->setImage(m_elements[index].pixmap);
        m_elements[index].cellWidget->setResolution(m_elements[index].pixmap.size());
        ui->table->resizeRowsToContents();
        m_elements[index].downloaded = true;
        m_imageUrl = QUrl(fileName);
        accept();
    }
}

void MovieImageDialog::onImageDropped(QUrl url)
{
    int index = m_elements.size();
    DownloadElement d;
    d.originalUrl = url;
    d.thumbUrl = url;
    d.downloaded = false;
    m_elements.append(d);
    this->renderTable();
    if (url.toString().startsWith("file://")) {
        m_elements[index].pixmap = QPixmap(url.toLocalFile());
        m_elements[index].pixmap = m_elements[index].pixmap.scaledToWidth(this->getColumnWidth()-10, Qt::SmoothTransformation);
        m_elements[index].cellWidget->setImage(m_elements[index].pixmap);
        m_elements[index].cellWidget->setResolution(m_elements[index].pixmap.size());
    }
    ui->table->resizeRowsToContents();
    m_elements[index].downloaded = true;
    m_imageUrl = url;
    accept();
}

void MovieImageDialog::onPreviewSizeChange(int value)
{
    ui->buttonZoomOut->setDisabled(value == ui->previewSizeSlider->minimum());
    ui->buttonZoomIn->setDisabled(value == ui->previewSizeSlider->maximum());
    QSettings settings;
    settings.setValue(QString("MovieImageDialog/PreviewSize_%1").arg(m_type), value);
    renderTable();
}

void MovieImageDialog::onZoomIn()
{
    ui->previewSizeSlider->setValue(ui->previewSizeSlider->value()+1);
}

void MovieImageDialog::onZoomOut()
{
    ui->previewSizeSlider->setValue(ui->previewSizeSlider->value()-1);
}
