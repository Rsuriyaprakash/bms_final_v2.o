/**
 * @file file_names.h
 * @author Suriya Prakash R
 * @version 2.0.0
 * @brief Centralized file and directory names for BMS persistence.
 */

#ifndef BMS_FILE_NAMES_H
#define BMS_FILE_NAMES_H

#ifdef __cplusplus
extern "C" {
#endif

#define BMS_DATA_DIRECTORY              "data"
#define BMS_REPORT_DIRECTORY            "reports"

#define BMS_USERS_FILE                  "data/users.dat"
#define BMS_DONORS_FILE                 "data/donors.dat"
#define BMS_HOSPITALS_FILE              "data/hospitals.dat"
#define BMS_INVENTORY_FILE              "data/inventory.dat"
#define BMS_REQUESTS_FILE               "data/blood_requests.dat"
#define BMS_DONATIONS_FILE              "data/donations.dat"
#define BMS_NOTIFICATIONS_FILE          "data/notifications.dat"
#define BMS_EMERGENCY_ALERTS_FILE       "data/emergency_alerts.dat"
#define BMS_HOSPITAL_GRAPH_FILE         "data/hospital_graph.dat"
#define BMS_DONATION_CAMPS_FILE          "data/donation_camps.dat"

#ifdef __cplusplus
}
#endif

#endif /* BMS_FILE_NAMES_H */
