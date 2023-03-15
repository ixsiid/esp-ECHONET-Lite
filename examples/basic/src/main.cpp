#include "esp_log.h"
#include "nvs_flash.h"

#include <wifiManager.hpp>
#include <udp-socket.hpp>
#include <pv-object.hpp>
#include <evps-object.hpp>
#include <battery-object.hpp>

#include "wifi_credential.h"
// #define SSID "your_ssid"
// #define WIFI_PASSWORD "your_wifi_password"

extern "C" {
void app_main();
}

void app_main(void) {
	static const char TAG[] = "PV";
	ESP_LOGI(TAG, "Start");

	esp_err_t ret;

	/* Initialize NVS — it is used to store PHY calibration data */
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	vTaskDelay(3 * 1000 / portTICK_PERIOD_MS);

	ret = WiFi::Connect(SSID, WIFI_PASSWORD);
	ESP_ERROR_CHECK(ret);

	ESP_LOGI(TAG, "IP: %s", WiFi::get_address());

	UDPSocket *udp = new UDPSocket();

	esp_ip_addr_t _multi;
	_multi.type		   = ESP_IPADDR_TYPE_V4;
	_multi.u_addr.ip4.addr = ipaddr_addr(ELConstant::EL_MULTICAST_IP);

	if (udp->beginReceive(ELConstant::EL_PORT)) {
		ESP_LOGI(TAG, "EL.udp.begin successful.");
	} else {
		ESP_LOGI(TAG, "Reseiver udp.begin failed.");	 // localPort
	}

	if (udp->beginMulticastReceive(&_multi)) {
		ESP_LOGI(TAG, "EL.udp.beginMulticast successful.");
	} else {
		ESP_LOGI(TAG, "Reseiver EL.udp.beginMulticast failed.");  // localPort
	}

	int packetSize;

	Profile * profile = new Profile(1, 13);

	PV *pv = new PV(1);
	EVPS *evps = new EVPS(3);
	Battery *battery = new Battery(4);

	profile->add(pv)
	       ->add(evps)
	       ->add(battery);

	evps->set_update_mode_cb([](EVPS::Mode current_mode, EVPS::Mode request_mode) {
		EVPS_Mode next_mode;
		switch (request_mode) {
			case EVPS::Mode::Charge:
				next_mode = EVPS::Mode::Charge;
				break;
			case EVPS::Mode::Stanby:
			case EVPS::Mode::Stop:
			case EVPS::Mode::Auto:
				next_mode = EVPS::Mode::Stanby;
				break;
			default:
				return EVPS::Mode::Unacceptable;
		}

		// モード繊維がないため、何もしない
		if (current_mode == next_mode) return current_mode;

		return next_mode;
	});

	uint8_t rBuffer[ELConstant::EL_BUFFER_SIZE];
	ELObject::elpacket_t *p = (ELObject::elpacket_t *)rBuffer;
	uint8_t *epcs		    = rBuffer + sizeof(ELObject::elpacket_t);

	esp_ip_addr_t remote_addr;

	const int pv_reset_count = 600;
	int pv_update_count		= pv_reset_count;

	while (true) {
		vTaskDelay(100 / portTICK_PERIOD_MS);
		if (--pv_update_count < 0) {
			pv_update_count = pv_reset_count;
			pv->update();
		}

		packetSize = udp->read(rBuffer, ELConstant::EL_BUFFER_SIZE, &remote_addr);

		if (packetSize > 0) {
			if (epcs[0] == 0xda || true) {
				ESP_LOGI("EL Packet", "(%04x, %04x) %04x-%02x -> %04x-%02x: ESV %02x [%02x]",
					    p->_1081, p->packet_id,
					    p->src_device_class, p->src_device_id,
					    p->dst_device_class, p->dst_device_id,
					    p->esv, p->epc_count);
				ESP_LOG_BUFFER_HEXDUMP("EL Packet", epcs, packetSize - sizeof(ELObject::elpacket_t), ESP_LOG_INFO);
			}

			uint8_t epc_res_count = 0;
			switch (p->dst_device_class) {
				case Profile::class_u16:
					epc_res_count = profile->process(p, epcs);
					if (epc_res_count > 0) {
						profile->send(udp, &remote_addr);
						continue;
					}
					break;
				case PV::class_u16:
					epc_res_count = pv->process(p, epcs);
					if (epc_res_count > 0) {
						pv->send(udp, &remote_addr);
						continue;
					}
					break;
				case EVPS::class_u16:
					epc_res_count = evps->process(p, epcs);
					if (epc_res_count > 0) {
						evps->send(udp, &remote_addr);
						continue;
					}
					break;
				case Battery::class_u16:
					epc_res_count = battery->process(p, epcs);
					if (epc_res_count > 0) {
						battery->send(udp, &remote_addr);
						continue;
					}
					break;
				default:
					ESP_LOGE(TAG, "Unknown class access: %hx", p->dst_device_class);
					break;
			}
		}
	}
}
