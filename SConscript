import os
from building import *

cwd = GetCurrentDir()
LOCAL_CPPPATH=[]

src = []
path = [cwd]
# add general drivers

src += Split("""stage/blog/blog.c""")
path += [cwd + r'/stage/blog']


src += Split("""
    stage/blfdt/src/fdt.c
    stage/blfdt/src/fdt_ro.c
    stage/blfdt/src/fdt_wip.c
    stage/blfdt/src/fdt_sw.c
    stage/blfdt/src/fdt_rw.c
    stage/blfdt/src/fdt_strerror.c
    stage/blfdt/src/fdt_empty_tree.c
    stage/blfdt/src/fdt_addresses.c
    stage/blfdt/src/fdt_overlay.c
""")

path += [
    cwd + r'/stage/blfdt/inc',
]

src += Split("""
    stage/blog/blog.c
    utils/src/utils_log.c
    utils/src/utils_hex.c
    utils/src/utils_dns.c
    utils/src/utils_getopt.c
    utils/src/utils_string.c
    utils/src/utils_crc.c
    utils/src/utils_list.c
    utils/src/utils_rbtree.c
    utils/src/utils_tlv_bl.c
""")
path += [
    cwd + r'/utils/include'
]

src += Split("""
    os/bl_os_adapter/bl_os_adapter/bl_os_hal.c
""")

src += Split("""
    security/blcrypto_suite/src/blcrypto_suite_bignum.c
    security/blcrypto_suite/src/blcrypto_suite_ecp.c
    security/blcrypto_suite/src/blcrypto_suite_ecp_curves.c
    security/blcrypto_suite/src/blcrypto_suite_platform_util.c
    security/blcrypto_suite/src/blcrypto_suite_porting.c
    security/blcrypto_suite/src/blcrypto_suite_hacc.c
    security/blcrypto_suite/src/blcrypto_suite_aes.c
    security/blcrypto_suite/src/blcrypto_suite_hacc_glue.c
    security/blcrypto_suite/src/blcrypto_suite_hacc_secp256r1_mul.c
    security/blcrypto_suite/src/blcrypto_suite_supplicant_api.c
    security/blcrypto_suite/src/blcrypto_suite_export_fw.c
""")

src += Split("""
    security/mbedtls_lts/port/mbedtls_port_mem.c
    security/mbedtls_lts/port/bignum_ext.c
    security/mbedtls_lts/port/test_case.c
""")
             
LOCAL_CPPPATH += [cwd + r'/security/mbedtls_lts',
         cwd + r'/security/mbedtls_lts/port']

src += Split("""
    network/wifi_bt_coex/src/wifi_bt_coex.c
    network/wifi_bt_coex/src/wifi_bt_coex_ctx.c
    network/wifi_bt_coex/src/wifi_bt_coex_impl_bl808.c
    network/wifi_hosal/wifi_hosal.c
    network/wifi_hosal/port/wifi_hosal_bl808.c
    network/wifi_manager/bl60x_wifi_driver/ipc_host.c
    network/wifi_manager/bl60x_wifi_driver/bl_cmds.c
    network/wifi_manager/bl60x_wifi_driver/bl_irqs.c
    network/wifi_manager/bl60x_wifi_driver/bl_main.c
    network/wifi_manager/bl60x_wifi_driver/bl_mod_params.c
    network/wifi_manager/bl60x_wifi_driver/bl_msg_rx.c
    network/wifi_manager/bl60x_wifi_driver/bl_msg_tx.c
    network/wifi_manager/bl60x_wifi_driver/bl_platform.c
    network/wifi_manager/bl60x_wifi_driver/bl_rx.c
    network/wifi_manager/bl60x_wifi_driver/bl_tx.c
    network/wifi_manager/bl60x_wifi_driver/bl_utils.c
    network/wifi_manager/bl60x_wifi_driver/stateMachine.c
    network/wifi_manager/bl60x_wifi_driver/wifi.c
    network/wifi_manager/bl60x_wifi_driver/wifi_mgmr.c
    network/wifi_manager/bl60x_wifi_driver/wifi_mgmr_api.c
    network/wifi_manager/bl60x_wifi_driver/wifi_mgmr_ext.c
    network/wifi_manager/bl60x_wifi_driver/wifi_mgmr_profile.c
    network/wifi_manager/bl60x_wifi_driver/wifi_mgmr_event.c
    network/wifi_manager/bl60x_wifi_driver/wifi_pkt_hooks.c
""")

src += Split("""
    security/wpa_supplicant/port/os_bl.c
    security/wpa_supplicant/src/ap/ap_config.c
    security/wpa_supplicant/src/ap/wpa_auth_ie.c
    security/wpa_supplicant/src/ap/wpa_auth_rsn_ccmp_only.c
    security/wpa_supplicant/src/bl_supplicant/bl_hostap.c
    security/wpa_supplicant/src/bl_supplicant/bl_wpa3.c
    security/wpa_supplicant/src/bl_supplicant/bl_wpa_main.c
    security/wpa_supplicant/src/bl_supplicant/bl_wpas_glue.c
    security/wpa_supplicant/src/bl_supplicant/bl_wps.c
    security/wpa_supplicant/src/common/sae.c
    security/wpa_supplicant/src/common/wpa_common.c
    security/wpa_supplicant/src/crypto/aes-cbc.c
    security/wpa_supplicant/src/crypto/aes-internal-bl.c
    security/wpa_supplicant/src/crypto/aes-omac1.c
    security/wpa_supplicant/src/crypto/aes-unwrap.c
    security/wpa_supplicant/src/crypto/aes-wrap.c
    security/wpa_supplicant/src/crypto/crypto_internal-modexp.c
    security/wpa_supplicant/src/crypto/dh_group5.c
    security/wpa_supplicant/src/crypto/dh_groups.c
    security/wpa_supplicant/src/crypto/md5-internal.c
    security/wpa_supplicant/src/crypto/md5.c
    security/wpa_supplicant/src/crypto/rc4.c
    security/wpa_supplicant/src/crypto/sha1-internal.c
    security/wpa_supplicant/src/crypto/sha1-pbkdf2.c
    security/wpa_supplicant/src/crypto/sha1.c
    security/wpa_supplicant/src/crypto/sha256-internal.c
    security/wpa_supplicant/src/crypto/sha256-prf.c
    security/wpa_supplicant/src/crypto/sha256.c
    security/wpa_supplicant/src/eap_peer/eap_common.c
    security/wpa_supplicant/src/rsn_supp/pmksa_cache.c
    security/wpa_supplicant/src/rsn_supp/wpa.c
    security/wpa_supplicant/src/rsn_supp/wpa_ie.c
    security/wpa_supplicant/src/utils/common.c
    security/wpa_supplicant/src/utils/wpa_debug.c
    security/wpa_supplicant/src/utils/wpabuf.c
    security/wpa_supplicant/src/wps/wps.c
    security/wpa_supplicant/src/wps/wps_attr_build.c
    security/wpa_supplicant/src/wps/wps_attr_parse.c
    security/wpa_supplicant/src/wps/wps_attr_process.c
    security/wpa_supplicant/src/wps/wps_common.c
    security/wpa_supplicant/src/wps/wps_dev_attr.c
    security/wpa_supplicant/src/wps/wps_enrollee.c
    security/wpa_supplicant/src/wps/wps_registrar.c
    security/wpa_supplicant/src/wps/wps_validate.c
    security/wpa_supplicant/test/test_crypto-bl.c
""")

path += [cwd + r'/stage/yloop/include']

path += [cwd + r'/os/bl_os_adapter/bl_os_adapter',
         cwd + r'/os/bl_os_adapter/bl_os_adapter/include',
         cwd + r'/os/bl_os_adapter/bl_os_adapter/include/bl_os_adapter']

path += [cwd + r'/network/wifi_manager/bl60x_wifi_driver']

path += [cwd + r'/network/wifi_bt_coex/include',
         cwd + r'/network/wifi_manager/bl60x_wifi_driver/include']

path += [cwd + r'/network/wifi_hosal/include']
path += [cwd + r'/network/wifi/include']


path += [cwd + r'/security/wpa_supplicant/port/include',
         cwd + r'/security/wpa_supplicant/src',
         cwd + r'/security/wpa_supplicant/include',
         cwd + r'/security/wpa_supplicant/include/bl_supplicant',
         cwd + r'/security/blcrypto_suite/inc',
         cwd + r'/security/blcrypto_suite/priv_inc',

         cwd + r'/fs/vfs/include']
   
src += Split("""
    fs/vfs/src/vfs.c
    fs/vfs/src/vfs_file.c
    fs/vfs/src/vfs_inode.c
    fs/vfs/src/vfs_register.c
    stage/yloop/src/aos_rtthread_port.c
    stage/yloop/src/yloop.c
    stage/yloop/src/select.c
    stage/yloop/src/device.c
    stage/yloop/src/local_event.c
""")
path += [cwd + r'/stage/yloop/include']
           
src += Split("""
    drv_wifi/bl_pm.c
    drv_wifi/bl_sec.c
    drv_wifi/bl_wifi.c
    drv_wifi/bl_pm.c
    drv_wifi/bl808_wifi.c
""")
path += [cwd + r'/drv_wifi']

libpath = [cwd + '/platform/soc/bl606p/bl606p_phyrf/lib']
libs = ['bl606p_phyrf.a']
libpath += [cwd + '/network/wifi/lib']
libs += ['libwifi.a']

LOCAL_CFLAGS = ' -DMBEDTLS_CONFIG_FILE="<mbedtls_sample_config.h>"'
CPPDEFINES = ['CFG_TXDESC="4"', 'CFG_STA_MAX="5"', 'CFG_CHIP_BL808', 'BL_CHIP_NAME="BL808"']
LOCAL_CFLAGS += ' -DARCH_RISCV'

group = DefineGroup('Libraries', src, depend = ['PKG_USING_WLAN_BL808'], CPPPATH = path, LIBS = libs, LIBPATH = libpath, CPPDEFINES = CPPDEFINES, LOCAL_CPPPATH = LOCAL_CPPPATH, LOCAL_CFLAGS = LOCAL_CFLAGS)


Return('group')
