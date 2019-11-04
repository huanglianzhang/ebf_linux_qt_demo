/******************************************************************
 Copyright (C) 2019 - All Rights Reserved by
 文 件 名 : weatherwidget.cpp --- WeatherWidget
 作 者    : Niyh(lynnhua)
 论 坛    : http://www.firebbs.cn
 编写日期 : 2019
 说 明    :
 历史纪录 :
 <作者>    <日期>        <版本>        <内容>
  Niyh	   2019    	1.0.0 1     文件创建
*******************************************************************/
#include "weatherwidget.h"
#include "citybook.h"
#include "skin.h"
#include "appconfig.h"

#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QDateTime>

#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRegExp>

#define REFRESH_HEIGHT      42

WeatherView::WeatherView(QWidget *parent) : QWidget(parent)
{
    m_bPressed = false;
    m_nYOffset = 0;
    m_bRefresh = false;
    m_strErrorMsg = "正在获取天气信息...";

    m_widgetRefresh = new QWidget(this);
    m_widgetRefresh->setStyleSheet(QString("QLabel{font-family: '%1'; font: 18px; color: #ffffff;}").arg(Skin::m_strAppFontNormal));
    m_widgetRefresh->setGeometry(0, - REFRESH_HEIGHT + m_nYOffset, this->width(), REFRESH_HEIGHT);

    m_btnRefesh = new QtRefreshButton(m_widgetRefresh);
    m_btnRefesh->SetPixmap(QPixmap(":/images/weather/ic_refresh.png"));
    m_btnRefesh->setFixedSize(42, 42);

    QHBoxLayout *horLayout = new QHBoxLayout(m_widgetRefresh);
    horLayout->setContentsMargins(0, 0, 0, 0);
    horLayout->setSpacing(10);
    horLayout->addStretch();
    horLayout->addWidget(m_btnRefesh);
    horLayout->addWidget(new QLabel(tr("正在刷新..."), m_widgetRefresh));
    horLayout->addStretch();

    m_btns.insert(0, new QtPixmapButton(0, QRect(0, 0, 0, 0), QPixmap(":/images/weather/ic_setting.png"), QPixmap(":/images/weather/ic_setting.png")));
    m_btns.insert(1, new QtPixmapButton(1, QRect(0, 0, 0, 0), QPixmap(":/images/weather/menu_icon.png"), QPixmap(":/images/weather/menu_icon_pressed.png")));
    m_pixmapWeatherType = QPixmap(":/images/weather/ic_weath_type.png");
    m_pixmapWeatherBackground = QPixmap(":/images/weather/background/多云.png");
}

WeatherView::~WeatherView()
{

}

void WeatherView::setWeatherData(const QStringList &weatherData)
{
    if (weatherData.size() < 5){
        showErrorMessage(tr("天气信息获取失败！！~"));
        return;
    }

    m_strErrorMsg = "";
    m_strWeatherDatas = weatherData;
    QString strType = weatherData.at(3);
    QPixmap pixmap = QPixmap(QString(":/images/weather/type/zh/%1.png").arg(strType));
    m_pixmapWeatherType = pixmap.isNull() ? m_pixmapWeatherType : pixmap;
    pixmap = QPixmap(QString(":/images/weather/background/%1.png").arg(strType));
    m_pixmapWeatherBackground = pixmap.isNull() ? m_pixmapWeatherBackground : pixmap;
    this->update();
}

void WeatherView::startRefresh()
{
    m_nYOffset = REFRESH_HEIGHT;
    m_widgetRefresh->setGeometry(0, -REFRESH_HEIGHT + m_nYOffset, this->width(), REFRESH_HEIGHT);
    m_btnRefesh->Start(2000);
    m_bRefresh = true;
    this->update();
    QTimer::singleShot(2000, this, SLOT(SltResetWidget()));
}

void WeatherView::showErrorMessage(const QString &text)
{
    m_strErrorMsg = text;
    this->update();
}

QString WeatherView::getTodayOfWeek()
{
    QString strWeek = "";
    if (m_strWeatherDatas.size() < 5) {
        int week = QDate::currentDate().dayOfWeek();
        if (1 == week) strWeek = QString("星期一");
        else if (2 == week) strWeek = QString("星期二");
        else if (3 == week) strWeek = QString("星期三");
        else if (4 == week) strWeek = QString("星期四");
        else if (5 == week) strWeek = QString("星期五");
        else if (6 == week) strWeek = QString("星期六");
        else if (7 == week) strWeek = QString("星期日");
    }
    else {
        strWeek = m_strWeatherDatas.at(2);
    }

    return strWeek;
}

void WeatherView::SltResetWidget()
{
    m_nYOffset = 0;
    m_bRefresh = false;
    m_widgetRefresh->setGeometry(0, - REFRESH_HEIGHT + m_nYOffset, this->width(), REFRESH_HEIGHT);
    this->update();
}

void WeatherView::resizeEvent(QResizeEvent *e)
{
    m_widgetRefresh->setGeometry(0, - REFRESH_HEIGHT + m_nYOffset, this->width(), REFRESH_HEIGHT);
    m_rectCenter = QRect(1, 1, this->width() - 2, this->height() - 26);
    m_btns.value(0)->setRect(QRect(15, 6, 42, 42));
    m_btns.value(1)->setRect(QRect(this->width() - 10 - 54, 1, 54, 54));
    QWidget::resizeEvent(e);
}

void WeatherView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawPixmap(this->rect(), m_pixmapWeatherBackground);

    drawWeatherInfo(&painter);
    drawStatusBar(&painter);
}

void WeatherView::drawWeatherInfo(QPainter *painter)
{
    painter->save();

    foreach (QtPixmapButton *btn, m_btns) {
        QRect rect(btn->rect().left(), btn->rect().top() + m_nYOffset, btn->rect().width(), btn->rect().height());
        painter->drawPixmap(rect, btn->pixmap());
    }

    painter->setPen(QColor("#ffffff"));
    QFont font(Skin::m_strAppFontBold);
    font.setPixelSize(28);
    font.setBold(true);
    painter->setFont(font);

    if (!m_strErrorMsg.isEmpty()) {
        painter->drawText(m_rectCenter, Qt::AlignCenter, m_strErrorMsg);
    }
    else {
        int nW = painter->fontMetrics().width(m_strWeatherDatas.at(0));
        painter->drawText((this->width() - nW) / 2, m_nYOffset + 10, nW, 25, Qt::AlignCenter, m_strWeatherDatas.at(0));
        font.setPixelSize(100);
        painter->setFont(font);
        QRect rectTemp((this->width() -  154 - 90) / 2, m_nYOffset + 35, 154, 140);
        painter->drawText(rectTemp, Qt::AlignCenter, m_strWeatherDatas.at(1) + "°");
//        painter->drawLine(QPoint(rectTemp.right(), rectTemp.top() + 20), rectTemp.bottomRight());

        QRect rectRight(rectTemp.right() + 10, rectTemp.top() + 25, m_pixmapWeatherType.width(), m_pixmapWeatherType.height());
        painter->drawPixmap(rectRight, m_pixmapWeatherType);

        font.setPixelSize(24);
        painter->setFont(font);
        rectRight = QRect(rectRight.left(), rectRight.bottom(), 200, 30);
        painter->drawText(rectRight, Qt::AlignVCenter, m_strWeatherDatas.at(3));

        rectRight = QRect(rectRight.left(), rectRight.bottom(), 200, 30);
        painter->drawText(rectRight, Qt::AlignVCenter, m_strWeatherDatas.at(4));
    }


    painter->restore();
}

void WeatherView::drawStatusBar(QPainter *painter)
{
    painter->save();
    QFont font(Skin::m_strAppFontBold);
    font.setPixelSize(18);
    painter->setFont(font);
    painter->setPen(QColor("#797979"));
    painter->drawText(25, this->height() - 25, 40, 25, Qt::AlignCenter, QString("预报"));
    painter->drawText(this->width() - 125, this->height() - 25, 40, 25, Qt::AlignCenter, QString("今天"));
    painter->setPen(QColor("#ffffff"));
    painter->drawText(this->width() - 85, this->height() - 25, 60, 25, Qt::AlignCenter, getTodayOfWeek());
    painter->restore();
}

void WeatherView::mousePressEvent(QMouseEvent *e)
{
    bool bOk = false;
    foreach (QtPixmapButton *btn, m_btns) {
        if (btn->rect().contains(e->pos())) {
            btn->setPressed(true);
            this->update();
            bOk = true;
            break;
        }
    }

    if (!bOk) {
        m_bPressed = true;
        m_startPos = e->pos();
    }
}

void WeatherView::mouseMoveEvent(QMouseEvent *e)
{
    if (m_bPressed && (e->y() > m_startPos.y()) && !m_bRefresh) {
        m_nYOffset += e->y() > m_startPos.y();
        if (m_nYOffset > 15){
            m_nYOffset = REFRESH_HEIGHT;
            m_btnRefesh->Start(2000);
            m_bRefresh = true;
            QTimer::singleShot(2000, this, SLOT(SltResetWidget()));
        }
        m_startPos = e->pos();
        m_widgetRefresh->setGeometry(0, - REFRESH_HEIGHT + m_nYOffset, this->width(), REFRESH_HEIGHT);
        this->update();
    }
}

void WeatherView::mouseReleaseEvent(QMouseEvent *e)
{
    foreach (QtPixmapButton *btn, m_btns) {
        if (btn->isPressed()) {
            btn->setPressed(false);
            emit signalToolBtnClicked(btn->id());
        }
    }

    if (m_nYOffset > 0 && !m_bRefresh) {
        SltResetWidget();
    }
    else if (m_bRefresh) {
        emit signalToolBtnClicked(2);
    }

    this->update();
    QWidget::mouseReleaseEvent(e);
}

///////////////////////////////////////////////////////////////////////
WeatherReport::WeatherReport(QWidget *parent) : QtListWidget(parent)
{
    this->SetBackground(Qt::transparent);
    m_bHorizontal = true;
    m_nItemSize = 150;
    m_nMargin = 10;
    m_nSpace = 10;
}

WeatherReport::~WeatherReport()
{

}

QPixmap WeatherReport::getPixmap(const QString &type)
{
    QPixmap pixmap = QPixmap(QString(":/images/weather/type/zh/%1.png").arg(type));
    return pixmap.isNull() ? QPixmap(":/images/weather/type/ic_weath_type.png") : pixmap;
}

void WeatherReport::drawItemInfo(QPainter *painter, QtListWidgetItem *item)
{
    painter->save();
    painter->setPen("#3f5c75");
    painter->setBrush(QColor("#00549c"));
    painter->drawRoundedRect(item->m_rect, 10, 10);

    QString strTemp = (item->m_strMultiParameters.size() > 2) ? item->m_strMultiParameters.at(0) : QDate::currentDate().toString("M月d日 ddd");
    painter->setPen(QColor("#ffffff"));
    QFont font(Skin::m_strAppFontNormal);
    font.setPixelSize(18);
    painter->setFont(font);
    QRect rectItem(item->m_rect.left(), item->m_rect.top() + 5, item->m_rect.width(), 25);
    painter->drawText(rectItem, Qt::AlignCenter, strTemp);

    strTemp = (item->m_strMultiParameters.size() > 2) ? item->m_strMultiParameters.at(1) : QString("天气晴");
    QPixmap pixmap = getPixmap(strTemp);
    int nX = (item->m_rect.width() - pixmap.width()) / 2;
    rectItem = QRect(rectItem.left() + nX, rectItem.bottom() + 10, pixmap.width(), pixmap.height());
    painter->drawPixmap(rectItem, pixmap);

    rectItem = QRect(item->m_rect.left(), rectItem.bottom() + 6, item->m_rect.width(), 25);
    painter->drawText(rectItem, Qt::AlignCenter, strTemp);

    strTemp = (item->m_strMultiParameters.size() > 2) ? item->m_strMultiParameters.at(2) : QString("18°~ 22°");
    font.setFamily(Skin::m_strAppFontBold);
    font.setPixelSize(22);
    painter->setFont(font);
    rectItem = QRect(item->m_rect.left(), rectItem.bottom() + 10, item->m_rect.width(), 25);
    painter->drawText(rectItem, Qt::AlignCenter, strTemp);
    painter->restore();
}

///////////////////////////////////////////////////////////////////////
WeatherWidget::WeatherWidget(QWidget *parent) : QtAnimationWidget(parent)
{
    this->SetBackground(QColor("#014b8a"));
    InitWidget();

    CityBook::Instance()->LoadConfig(qApp->applicationDirPath() + "/conf/city.json");
    // 默认北京市
    m_strCityCode = AppConfig::ReadSetting("Weathcer", "city", "101010100").toString();
    m_cityManager = NULL;
    // 天气api查询
    m_weatherApi = new WeatherApi(this);
    connect(m_weatherApi, SIGNAL(signalResult(QByteArray)), this, SLOT(SltWeatherReply(QByteArray)));

    // 添加默认数据
    for (int i = 0; i < 7; i++) {
        m_weatherItems.insert(i, new QtListWidgetItem(i,
                                                      QString("雷阵雨"),
                                                      QString("27°~ %1°").arg(30 + i),
                                                      QPixmap(":/images/weather/ic_weath_type.png")));
    }
    m_weatherReport->SetItems(m_weatherItems);


    QTimer::singleShot(500, this, SLOT(InitWeather()));
}

WeatherWidget::~WeatherWidget()
{

}

void WeatherWidget::InitWidget()
{
    m_weatherView = new WeatherView(this);
    m_weatherView->setMinimumHeight(250);
    connect(m_weatherView, SIGNAL(signalToolBtnClicked(int)), this, SLOT(SltToolBtnClicked(int)));

    m_weatherReport = new WeatherReport(this);

    QWidget *widgetBottom = new QWidget(this);
    widgetBottom->setMaximumHeight(30);
    widgetBottom->setObjectName("widgetBottom");
    widgetBottom->setStyleSheet(QString("QWidget#widgetBottom {border: none; border-top: 1px solid #87aaaaaa;}"
                                        "QLabel {font-family: '%1'; font: 18px; color: #7dffffff;}").arg(Skin::m_strAppFontBold));
    QHBoxLayout *horLayoutBottom = new QHBoxLayout(widgetBottom);
    horLayoutBottom->setContentsMargins(24, 1, 24, 5);
    horLayoutBottom->setSpacing(10);
    horLayoutBottom->addWidget(new QLabel(tr("中央气象台"), widgetBottom));
    horLayoutBottom->addStretch();

    m_labelDate = new QLabel(this);
    m_labelDate->setText(tr("16:11 发布"));
    horLayoutBottom->addWidget(m_labelDate);

    QVBoxLayout *verLayoutAll = new QVBoxLayout(this);
    verLayoutAll->setContentsMargins(0, 0, 0, 0);
    verLayoutAll->setSpacing(0);
    verLayoutAll->addWidget(m_weatherView);
    verLayoutAll->addWidget(m_weatherReport, 1);
    verLayoutAll->addWidget(widgetBottom);
}

QString WeatherWidget::getTemperature(const QString &high, const QString &low)
{
    QString strResult = "";
    QRegExp regExp("(\\d+)");
    if (-1 != regExp.indexIn(low)) {
        strResult += QString("%1°") .arg(regExp.cap(1).toInt());
    }

    if (-1 != regExp.indexIn(high)) {
        strResult += QString("~ %1° ") .arg(regExp.cap(1).toInt());
    }

    return strResult;
}

void WeatherWidget::SltCityManagerBack()
{
    if (NULL != m_cityManager) {
        m_cityManager->StartAnimation(QPoint(0, 0), QPoint(0, this->height()), 300, false);
    }
}

void WeatherWidget::SltCityChanged(const QString &city)
{
    m_strCityCode = CityBook::Instance()->GetCityId(city);
    AppConfig::SaveSetting("Weathcer", "city", m_strCityCode);;
    InitWeather();
    SltCityManagerBack();
}

void WeatherWidget::InitWeather()
{
    if (m_strCityCode.isEmpty()) {
        m_weatherView->showErrorMessage(tr("暂无当前城市天气信息!"));
        return;
    }

    m_weatherView->startRefresh();
    m_weatherApi->QueryCityWeather(m_strCityCode);
}

void WeatherWidget::SltToolBtnClicked(int index)
{
    if (0 == index) {
        if (NULL == m_cityManager) {
            m_cityManager = new CityManagerWidget(this);
        }

        m_cityManager->resize(this->size());
        connect(m_cityManager, SIGNAL(signalBackHome()), this, SLOT(SltCityManagerBack()));
        connect(m_cityManager, SIGNAL(signalCityChanged(QString)), this, SLOT(SltCityChanged(QString)));
        m_cityManager->setVisible(true);
        m_cityManager->StartAnimation(QPoint(0, this->height()), QPoint(0, 0), 200, true);
    }
    else if (1 == index) {
        emit signalBackHome();
    }
    else if (2 == index) {
        InitWeather();
    }
}

void WeatherWidget::SltWeatherReply(const QByteArray &jsonData)
{
    bool bOk = false;
    QtJson::JsonObject result = QtJson::parse(jsonData, bOk).toMap();
    if (bOk) {
        QStringList strTodayData;
        QtJson::JsonObject jsonCity = result.value("cityInfo").toMap();
        strTodayData << jsonCity.value("city").toString();
        QtJson::JsonObject jsonCityData = result.value("data").toMap();
        strTodayData << jsonCityData.value("wendu").toString();
        m_labelDate->setText(jsonCity.value("updateTime").toString() + "发布");
        QtJson::JsonArray jsonForecast = jsonCityData.value("forecast").toList();

        int nMonty = QDate::currentDate().month();
        for (int i = 0; i < jsonForecast.size(); i++) {
            QtJson::JsonObject jsonObj = jsonForecast.at(i).toMap();
            QStringList strForeacast;
            int day = jsonObj.value("date").toString().toInt();
            strForeacast << (QString("%1月%2日 ").arg(nMonty).arg(day) + jsonObj.value("week").toString());
            QString strType = jsonObj.value("type").toString();
            strForeacast << strType;
            strForeacast << getTemperature(jsonObj.value("high").toString(), jsonObj.value("low").toString());

            // 添加当天的天气
            if (0 == i) {
                strTodayData << jsonObj.value("week").toString();
                strTodayData << strType;
                strTodayData << getTemperature(jsonObj.value("high").toString(), jsonObj.value("low").toString());
                m_weatherView->setWeatherData(strTodayData);
            }

            m_weatherItems.insert(i, new QtListWidgetItem(i, strForeacast));
        }

        m_weatherReport->SetItems(m_weatherItems);
    }
    else {
        qDebug() << "get weather failed";
    }
}