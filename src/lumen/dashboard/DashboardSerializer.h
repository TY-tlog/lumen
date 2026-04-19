#pragma once

#include <QJsonObject>

namespace lumen::dashboard {

class Dashboard;

class DashboardSerializer {
public:
    static QJsonObject toJson(const Dashboard& dashboard);
    static void fromJson(const QJsonObject& obj, Dashboard& dashboard);

    static QJsonObject migrateV1toV2(const QJsonObject& v1Root);
};

}  // namespace lumen::dashboard
