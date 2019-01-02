#ifndef IOT_TASK_3_SERVICES_H
#define IOT_TASK_3_SERVICES_H

void processLedRequest(int serviceData);
void LED_SOS();
void processBuzzerRequest(int serviceData);
void buzzSiren();
void buzzFanfare();
void processFanRequest(int serviceData);
void processRgbLedRequest(int serviceData);
void rgbParty();

void flashScreen();

#endif //IOT_TASK_3_SERVICES_H
