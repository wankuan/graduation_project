adb root
adb remount
adb push backstage_service/inner_service/inner_service.out /data/graduation_project
adb push app_service/test_case/app_running/app_test.out /data/graduation_project
adb push tank_components/bsp/uart/test_uart.out /data/graduation_project