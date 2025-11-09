#pragma once

#include <qobject.h>
#include <qfile.h>
#include <qurl.h>
#include <qdir.h>
#include <qfilesystemwatcher.h>
#include <qqmlintegration.h>
#include <qiodevice.h>
#include <qtimer.h>
#include <QAbstractItemModel>

class BrightnessEntry : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(qreal current           READ current           WRITE current);
    Q_PROPERTY(qreal currentNormalized READ currentNormalized WRITE currentNormalized);
    Q_PROPERTY(qreal max               READ max);

    Q_PROPERTY(QString id              READ id);

    QML_UNCREATABLE("Only I get to create them lmao");
    QML_NAMED_ELEMENT(BrightnessEntryBase);

    friend class Brightness;

public:
    using Id_t = QString;

    enum class Class
    {
        Backlight,
        Led
    };

    BrightnessEntry(
        QObject* parent,
        Class clazz,
        const Id_t& id,
        int current,
        int max,
        const QString& path_current);

    // Backlight defaults
    [[nodiscard]] int current() const;
    void current(int value);

    [[nodiscard]] qreal currentNormalized() const;
    void currentNormalized(qreal value);

    [[nodiscard]] int max() const;

    Class clazz() const;
    const QString& id() const;
    const QString& pathCurrent() const;

    static QString classAsString(Class clazz);

private:
    struct Sync
    {
        qsizetype timeoutUntil = 0;
        int queuedValue = 0;
        QTimer* timer = nullptr;
    } _sync;

    Class _class;
    Id_t _id;

    int _current;
    int _max;

    QString _pathCurrent;

    int _updateDelay = 50;

    void requestSync();

    void writeChanges();
};

class Brightness : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(int updateDelay READ updateDelay WRITE updateDelay)

    Q_PROPERTY(qreal backlight           READ backlight           WRITE backlight)
    Q_PROPERTY(qreal backlightNormalized READ backlightNormalized WRITE backlightNormalized)
    Q_PROPERTY(qreal backlightMax        READ backlightMax)

    Q_PROPERTY(QList<BrightnessEntry*> backlights READ backlights)
    Q_PROPERTY(QList<BrightnessEntry*> leds       READ leds)

    QML_SINGLETON
    QML_NAMED_ELEMENT(BrightnessController);

public:
    explicit Brightness(QObject* parent = nullptr);

    [[nodiscard]] int updateDelay() const;
    void updateDelay(int value);

    [[nodiscard]] int backlight() const;
    void backlight(int value);

    [[nodiscard]] qreal backlightNormalized() const;
    void backlightNormalized(qreal value);

    [[nodiscard]] int backlightMax() const;

    [[nodiscard]] QList<BrightnessEntry*> backlights();
    [[nodiscard]] QList<BrightnessEntry*> leds();

private:
    static QList<BrightnessEntry*> parseClass(Brightness* thiz, BrightnessEntry::Class clazz);

    int _updateDelay = 50;
    QFileSystemWatcher _watcher;

    QList<BrightnessEntry*> _backlights;
    QList<BrightnessEntry*> _leds;
};