#include "document.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#ifdef Q_OS_ANDROID
#include "androidhelper.h"
#else
#include <quazip5/quazip.h>
#include <quazip5/quazipfile.h>
#endif

Document::Document(QUndoStack *undoStack, QObject *parent)
    : QObject(parent), mUndoStack(undoStack) {}

void Document::newDocument() {
  setMeasurements();
  setMapImageUrl(QUrl(":/A4_120dpi.png"));
  setNeedsSaving(false);
}

bool Document::save(QUrl fileUrl) {
#ifndef Q_OS_ANDROID
  QuaZip zip(fileUrl.toLocalFile());
  if (!zip.open(QuaZip::Mode::mdCreate)) {
    return false;
  }

  QJsonObject root;
  QJsonArray data;
  for (auto position : mMeasurements->positions()) {
    QJsonObject point;
    point["x"] = position.pos.x();
    point["y"] = position.pos.y();
    QJsonArray scanInfo;
    QJsonArray iperfRxInfo;
    QJsonArray iperfTxInfo;
    QJsonArray iperfRetransmitsInfo;

    for (auto s : mMeasurements->measurementsAt(position)) {
      QJsonObject scanItem;
      scanItem["bssid"] = s.bss.bssid;
      scanItem["ssid"] = s.bss.ssid;
      scanItem["freq"] = s.bss.freq;
      scanItem["channel"] = s.bss.channel;
      switch (s.measurementType) {
      case WiFiSignal:
        scanItem["signal"] = s.value;
        scanInfo.append(scanItem);
        break;
      case IperfRx:
        scanItem["value"] = s.value;
        iperfRxInfo.append(scanItem);
        break;
      case IperfTx:
        scanItem["value"] = s.value;
        iperfTxInfo.append(scanItem);
        break;
      case IperfRetransmits:
        scanItem["value"] = s.value;
        iperfRetransmitsInfo.append(scanItem);
        break;
      }
    }
    point["scanInfo"] = scanInfo;
    point["iperfRx"] = iperfRxInfo;
    point["iperfTxInfo"] = iperfTxInfo;
    point["iperfRetransmitsInfo"] = iperfRetransmitsInfo;

    data.append(point);
  }
  root["data"] = data;

  QJsonDocument jdoc(root);

  QuaZipFile dataFile(&zip);
  dataFile.open(QIODevice::WriteOnly, QuaZipNewInfo("data.json"));
  dataFile.write(jdoc.toJson(QJsonDocument::Compact));
  dataFile.close();

  QuaZipFile mapfile(&zip);
  mapfile.open(QIODevice::WriteOnly, QuaZipNewInfo("mapimage.png"));
  mMapImage.save(&mapfile, "png");
  mapfile.close();

  zip.close();

  setNeedsSaving(false);
#endif

  return true;
}

void Document::load(QUrl fileUrl) {
#ifndef Q_OS_ANDROID
  QuaZip zip(fileUrl.toLocalFile());
  if (!zip.open(QuaZip::Mode::mdUnzip)) {
    return;
  }
  setMeasurements();

  QuaZipFile file(&zip);
  for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile()) {
    file.open(QIODevice::ReadOnly);
    if (file.getActualFileName() == "data.json") {
      read(file.readAll());
    } else if (file.getActualFileName() == "mapimage.png") {
      QImage image;
      image.load(&file, "png");
      setMapImage(image);
    }
    file.close();
  }
  zip.close();
#endif
  setNeedsSaving(false);
}

Measurements *Document::measurements() const { return mMeasurements.get(); }

void helper(const QJsonObject &point, QString name,
            MeasurementType measurementType, QVector<MeasurementEntry> &scans) {
  QJsonArray iperfRxInfo = point[name].toArray();
  for (int j = 0; j < iperfRxInfo.size(); ++j) {
    QJsonObject scanItem = iperfRxInfo[j].toObject();
    if (scanItem.contains("bssid") && scanItem["bssid"].isString() &&
        scanItem.contains("ssid") && scanItem["ssid"].isString() &&
        scanItem.contains("value") && scanItem["value"].isDouble() &&
        scanItem.contains("freq") && scanItem["freq"].isDouble() &&
        scanItem.contains("channel") && scanItem["channel"].isDouble()) {

      Bss bss{scanItem["bssid"].toString(), scanItem["ssid"].toString(),
              scanItem["freq"].toInt(), scanItem["channel"].toInt()};
      scans.push_back({bss, measurementType, scanItem["value"].toDouble()});
    }
  }
}

void Document::read(QByteArray data) {
  QJsonDocument jdoc(QJsonDocument::fromJson(data));
  QJsonObject root = jdoc.object();

  if (root.contains("data") && root["data"].isArray()) {
    QJsonArray data = root["data"].toArray();
    for (int i = 0; i < data.size(); ++i) {
      QJsonObject point = data[i].toObject();
      if (point.contains("x") && point["x"].isDouble() && point.contains("y") &&
          point["y"].isDouble() && point.contains("scanInfo") &&
          point["scanInfo"].isArray()) {

        QVector<MeasurementEntry> scans;

        helper(point, "iperfRx", IperfRx, scans);
        helper(point, "iperfTxInfo", IperfTx, scans);
        helper(point, "iperfRetransmitsInfo", IperfRetransmits, scans);

        QJsonArray scanInfo = point["scanInfo"].toArray();
        for (int j = 0; j < scanInfo.size(); ++j) {
          QJsonObject scanItem = scanInfo[j].toObject();
          if (scanItem.contains("bssid") && scanItem["bssid"].isString() &&
              scanItem.contains("ssid") && scanItem["ssid"].isString() &&
              scanItem.contains("signal") && scanItem["signal"].isDouble() &&
              scanItem.contains("freq") && scanItem["freq"].isDouble() &&
              scanItem.contains("channel") && scanItem["channel"].isDouble()) {

            Bss bss{scanItem["bssid"].toString(), scanItem["ssid"].toString(),
                    scanItem["freq"].toInt(), scanItem["channel"].toInt()};
            scans.push_back({bss, WiFiSignal, scanItem["signal"].toDouble()});
          }
        }

        auto position =
            Position{QPoint(point["x"].toInt(), point["y"].toInt())};
        mMeasurements->addPosition(position);
        mMeasurements->newMeasurementsAtPosition(position, scans);
      }
    }
  }
}

QImage Document::mapImage() const { return mMapImage; }

void Document::setMapImage(const QImage &mapImage) {
  mMapImage = mapImage;
  emit mapImageChanged();
}

void Document::setMeasurements() {
  mMeasurements.reset(new Measurements());
  mUndoStack->clear();

  // FIXME:
  if (mMeasurements) {
    connect(mMeasurements.get(), &Measurements::postMeasurementAppended, this,
            [=]() { setNeedsSaving(true); });
    connect(mMeasurements.get(), &Measurements::postMeasurementRemoved, this,
            [=]() {
              if (mMeasurements->positions().size() == 0)
                setNeedsSaving(false);
              else
                setNeedsSaving(true);
            });
    connect(mMeasurements.get(), &Measurements::positionChanged, this,
            [=]() { setNeedsSaving(true); });
  }

  measurementsChanged(mMeasurements.get());
}

void Document::setNeedsSaving(bool value) {
  if (mNeedsSaving != value) {
    mNeedsSaving = value;
    emit needsSavingChanged(mNeedsSaving);
  }
}

void Document::setMapImageUrl(const QUrl &mapImageUrl) {
#ifdef Q_OS_ANDROID
  if (mapImageUrl.scheme() == "content") {
    if (!checkPermission("READ_EXTERNAL_STORAGE")) {
      return;
    }
    setMapImage(imageFromContentUrl(mapImageUrl));
  } else {
    setMapImage(QImage(mapImageUrl.path()));
  }
#else
  setMapImage(QImage(mapImageUrl.path()));
#endif
}

bool Document::needsSaving() const { return mNeedsSaving; }
