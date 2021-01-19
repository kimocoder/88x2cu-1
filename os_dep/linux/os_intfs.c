/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#define _OS_INTFS_C_

#include <drv_types.h>
#include <hal_data.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Realtek Wireless Lan Driver");
MODULE_AUTHOR("Realtek Semiconductor Corp.");
MODULE_VERSION(DRIVERVERSION);

int netdev_open(struct net_device *pnetdev);
static int netdev_close(struct net_device *pnetdev);

/**
 * rtw_net_set_mac_address
 * This callback function is used for the Media Access Control address
 * of each net_device needs to be changed.
 *
 * Arguments:
 * @pnetdev: net_device pointer.
 * @addr: new MAC address.
 *
 * Return:
 * ret = 0: Permit to change net_device's MAC address.
 * ret = -1 (Default): Operation not permitted.
 *
 * Auther: Arvin Liu
 * Date: 2015/05/29
 */
static int rtw_net_set_mac_address(struct net_device *pnetdev, void *addr)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct sockaddr *sa = (struct sockaddr *)addr;
	int ret = -1;

	/* only the net_device is in down state to permit modifying mac addr */
	if ((pnetdev->flags & IFF_UP) == _TRUE) {
		RTW_INFO(FUNC_ADPT_FMT": The net_device's is not in down state\n"
			 , FUNC_ADPT_ARG(padapter));

		return ret;
	}

	/* if the net_device is linked, it's not permit to modify mac addr */
	if (check_fwstate(pmlmepriv, WIFI_UNDER_LINKING) ||
	    check_fwstate(pmlmepriv, WIFI_ASOC_STATE) ||
	    check_fwstate(pmlmepriv, WIFI_UNDER_SURVEY)) {
		RTW_INFO(FUNC_ADPT_FMT": The net_device's is not idle currently\n"
			 , FUNC_ADPT_ARG(padapter));

		return ret;
	}

	/* check whether the input mac address is valid to permit modifying mac addr */
	if (rtw_check_invalid_mac_address(sa->sa_data, _FALSE) == _TRUE) {
		RTW_INFO(FUNC_ADPT_FMT": Invalid Mac Addr for "MAC_FMT"\n"
			 , FUNC_ADPT_ARG(padapter), MAC_ARG(sa->sa_data));

		return ret;
	}

	_rtw_memcpy(adapter_mac_addr(padapter), sa->sa_data, ETH_ALEN); /* set mac addr to adapter */
	_rtw_memcpy(pnetdev->dev_addr, sa->sa_data, ETH_ALEN); /* set mac addr to net_device */

	rtw_hal_set_hw_macaddr(padapter, sa->sa_data);

	RTW_INFO(FUNC_ADPT_FMT": Set Mac Addr to "MAC_FMT" Successfully\n"
		 , FUNC_ADPT_ARG(padapter), MAC_ARG(sa->sa_data));

	ret = 0;

	return ret;
}

static struct net_device_stats *rtw_net_get_stats(struct net_device *pnetdev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct xmit_priv *pxmitpriv = &(padapter->xmitpriv);
	struct recv_priv *precvpriv = &adapter_to_dvobj(padapter)->recvpriv;

	padapter->stats.tx_packets = pxmitpriv->tx_pkts;/* pxmitpriv->tx_pkts++; */
	padapter->stats.rx_packets = precvpriv->rx_pkts;/* precvpriv->rx_pkts++; */
	padapter->stats.tx_dropped = pxmitpriv->tx_drop;
	padapter->stats.rx_dropped = precvpriv->rx_drop;
	padapter->stats.tx_bytes = pxmitpriv->tx_bytes;
	padapter->stats.rx_bytes = precvpriv->rx_bytes;

	return &padapter->stats;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
/*
 * AC to queue mapping
 *
 * AC_VO -> queue 0
 * AC_VI -> queue 1
 * AC_BE -> queue 2
 * AC_BK -> queue 3
 */
static const u16 rtw_1d_to_queue[8] = { 2, 3, 3, 2, 1, 1, 0, 0 };

/* Given a data frame determine the 802.1p/1d tag to use. */
unsigned int rtw_classify8021d(struct sk_buff *skb)
{
	unsigned int dscp;

	/* skb->priority values from 256->263 are magic values to
	 * directly indicate a specific 802.1d priority.  This is used
	 * to allow 802.1d priority to be passed directly in from VLAN
	 * tags, etc.
	 */
	if (skb->priority >= 256 && skb->priority <= 263)
		return skb->priority - 256;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		dscp = ip_hdr(skb)->tos & 0xfc;
		break;
	default:
		return 0;
	}

	return dscp >> 5;
}


static u16 rtw_select_queue(struct net_device *dev, struct sk_buff *skb
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)
	#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
	, struct net_device *sb_dev
	#else
	, void *accel_priv
	#endif
	#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)) && (LINUX_VERSION_CODE < KERNEL_VERSION(5, 2, 0)))
	, select_queue_fallback_t fallback
	#endif
#endif
)
{
	_adapter	*padapter = rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	skb->priority = rtw_classify8021d(skb);

	if (pmlmepriv->acm_mask != 0)
		skb->priority = qos_acm(pmlmepriv->acm_mask, skb->priority);

	return rtw_1d_to_queue[skb->priority];
}
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)) */

u16 rtw_os_recv_select_queue(u8 *msdu, enum rtw_rx_llc_hdl llc_hdl)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	u32 priority = 0;

	if (llc_hdl == RTW_RX_LLC_REMOVE) {
		u16 eth_type = RTW_GET_BE16(msdu + SNAP_SIZE);

		if (eth_type == ETH_P_IP) {
			struct iphdr *iphdr = (struct iphdr *)(msdu + SNAP_SIZE + 2);
			unsigned int dscp = iphdr->tos & 0xfc;

			priority = dscp >> 5;
		}
	}

	return rtw_1d_to_queue[priority];
#else
	return 0;
#endif
}

static u8 is_rtw_ndev(struct net_device *ndev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
	return ndev->netdev_ops
		&& ndev->netdev_ops->ndo_do_ioctl
		&& ndev->netdev_ops->ndo_do_ioctl == rtw_ioctl;
#else
	return ndev->do_ioctl
		&& ndev->do_ioctl == rtw_ioctl;
#endif
}

static int rtw_ndev_notifier_call(struct notifier_block *nb, unsigned long state, void *ptr)
{
	struct net_device *ndev;

	if (ptr == NULL)
		return NOTIFY_DONE;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 11, 0))
	ndev = netdev_notifier_info_to_dev(ptr);
#else
	ndev = ptr;
#endif

	if (ndev == NULL)
		return NOTIFY_DONE;

	if (!is_rtw_ndev(ndev))
		return NOTIFY_DONE;

	RTW_INFO(FUNC_NDEV_FMT" state:%lu\n", FUNC_NDEV_ARG(ndev), state);

	switch (state) {
	case NETDEV_CHANGENAME:
		rtw_adapter_proc_replace(ndev);
		break;
	case NETDEV_PRE_UP :
		{
			_adapter *adapter = rtw_netdev_priv(ndev);

			rtw_pwr_wakeup(adapter);
		}
		break;
	}

	return NOTIFY_DONE;
}

static struct notifier_block rtw_ndev_notifier = {
	.notifier_call = rtw_ndev_notifier_call,
};

int rtw_ndev_notifier_register(void)
{
	return register_netdevice_notifier(&rtw_ndev_notifier);
}

void rtw_ndev_notifier_unregister(void)
{
	unregister_netdevice_notifier(&rtw_ndev_notifier);
}

int rtw_ndev_init(struct net_device *dev)
{
	_adapter *adapter = rtw_netdev_priv(dev);

	RTW_PRINT(FUNC_ADPT_FMT" if%d mac_addr="MAC_FMT"\n"
		, FUNC_ADPT_ARG(adapter), (adapter->iface_id + 1), MAC_ARG(dev->dev_addr));
	strncpy(adapter->old_ifname, dev->name, IFNAMSIZ);
	adapter->old_ifname[IFNAMSIZ - 1] = '\0';
	rtw_adapter_proc_init(dev);

	return 0;
}

void rtw_ndev_uninit(struct net_device *dev)
{
	_adapter *adapter = rtw_netdev_priv(dev);

	RTW_PRINT(FUNC_ADPT_FMT" if%d\n"
		  , FUNC_ADPT_ARG(adapter), (adapter->iface_id + 1));
	rtw_adapter_proc_deinit(dev);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
static const struct net_device_ops rtw_netdev_ops = {
	.ndo_init = rtw_ndev_init,
	.ndo_uninit = rtw_ndev_uninit,
	.ndo_open = netdev_open,
	.ndo_stop = netdev_close,
	.ndo_start_xmit = rtw_xmit_entry,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	.ndo_select_queue	= rtw_select_queue,
#endif
	.ndo_set_mac_address = rtw_net_set_mac_address,
	.ndo_get_stats = rtw_net_get_stats,
	.ndo_do_ioctl = rtw_ioctl,
};
#endif

int rtw_init_netdev_name(struct net_device *pnetdev, const char *ifname)
{
#ifdef CONFIG_EASY_REPLACEMENT
	_adapter *padapter = rtw_netdev_priv(pnetdev);
	struct net_device	*TargetNetdev = NULL;
	_adapter			*TargetAdapter = NULL;

	if (padapter->bDongle == 1) {
		TargetNetdev = rtw_get_same_net_ndev_by_name(pnetdev, "wlan0");
		if (TargetNetdev) {
			RTW_INFO("Force onboard module driver disappear !!!\n");
			TargetAdapter = rtw_netdev_priv(TargetNetdev);
			TargetAdapter->DriverState = DRIVER_DISAPPEAR;

			padapter->pid[0] = TargetAdapter->pid[0];
			padapter->pid[1] = TargetAdapter->pid[1];
			padapter->pid[2] = TargetAdapter->pid[2];

			dev_put(TargetNetdev);
			unregister_netdev(TargetNetdev);

			padapter->DriverState = DRIVER_REPLACE_DONGLE;
		}
	}
#endif /* CONFIG_EASY_REPLACEMENT */

	if (dev_alloc_name(pnetdev, ifname) < 0)
		RTW_ERR("dev_alloc_name, fail!\n");

	rtw_netif_carrier_off(pnetdev);
	/* rtw_netif_stop_queue(pnetdev); */

	return 0;
}

void rtw_hook_if_ops(struct net_device *ndev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
	ndev->netdev_ops = &rtw_netdev_ops;
#else
	ndev->init = rtw_ndev_init;
	ndev->uninit = rtw_ndev_uninit;
	ndev->open = netdev_open;
	ndev->stop = netdev_close;
	ndev->hard_start_xmit = rtw_xmit_entry;
	ndev->set_mac_address = rtw_net_set_mac_address;
	ndev->get_stats = rtw_net_get_stats;
	ndev->do_ioctl = rtw_ioctl;
#endif
}

#ifdef CONFIG_CONCURRENT_MODE
static void rtw_hook_vir_if_ops(struct net_device *ndev);
#endif
struct net_device *rtw_init_netdev(_adapter *old_padapter)
{
	_adapter *padapter;
	struct net_device *pnetdev;

	if (old_padapter != NULL) {
		rtw_os_ndev_free(old_padapter);
		pnetdev = rtw_alloc_etherdev_with_old_priv(sizeof(_adapter), (void *)old_padapter);
	} else
		pnetdev = rtw_alloc_etherdev(sizeof(_adapter));

	if (!pnetdev)
		return NULL;

	padapter = rtw_netdev_priv(pnetdev);
	padapter->pnetdev = pnetdev;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
	SET_MODULE_OWNER(pnetdev);
#endif

	rtw_hook_if_ops(pnetdev);
#ifdef CONFIG_CONCURRENT_MODE
	if (!is_primary_adapter(padapter))
		rtw_hook_vir_if_ops(pnetdev);
#endif /* CONFIG_CONCURRENT_MODE */


#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
        pnetdev->features |= (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
        pnetdev->hw_features |= (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM);
#endif
#endif

#ifdef CONFIG_RTW_NETIF_SG
        pnetdev->features |= NETIF_F_SG;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
        pnetdev->hw_features |= NETIF_F_SG;
#endif
#endif

	if ((pnetdev->features & NETIF_F_SG) && (pnetdev->features & NETIF_F_IP_CSUM)) {
		pnetdev->features |= (NETIF_F_TSO | NETIF_F_GSO);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 39)
		pnetdev->hw_features |= (NETIF_F_TSO | NETIF_F_GSO);
#endif
	}
	/* pnetdev->tx_timeout = NULL; */
	pnetdev->watchdog_timeo = HZ * 3; /* 3 second timeout */

#ifdef CONFIG_WIRELESS_EXT
	pnetdev->wireless_handlers = (struct iw_handler_def *)&rtw_handlers_def;
#endif

#ifdef WIRELESS_SPY
	/* priv->wireless_data.spy_data = &priv->spy_data; */
	/* pnetdev->wireless_data = &priv->wireless_data; */
#endif

	return pnetdev;
}

int rtw_os_ndev_alloc(_adapter *adapter)
{
	int ret = _FAIL;
	struct net_device *ndev = NULL;

	ndev = rtw_init_netdev(adapter);
	if (ndev == NULL) {
		rtw_warn_on(1);
		goto exit;
	}
#if LINUX_VERSION_CODE > KERNEL_VERSION(2, 5, 0)
	SET_NETDEV_DEV(ndev, dvobj_to_dev(adapter_to_dvobj(adapter)));
#endif

#ifdef CONFIG_PCI_HCI
	if (adapter_to_dvobj(adapter)->bdma64)
		ndev->features |= NETIF_F_HIGHDMA;
	ndev->irq = adapter_to_dvobj(adapter)->irq;
#endif

#if defined(CONFIG_IOCTL_CFG80211)
	if (rtw_cfg80211_ndev_res_alloc(adapter) != _SUCCESS) {
		rtw_warn_on(1);
	} else
#endif
	ret = _SUCCESS;

	if (ret != _SUCCESS && ndev)
		rtw_free_netdev(ndev);
exit:
	return ret;
}

void rtw_os_ndev_free(_adapter *adapter)
{
#if defined(CONFIG_IOCTL_CFG80211)
	rtw_cfg80211_ndev_res_free(adapter);
#endif

	if (adapter->pnetdev) {
		rtw_free_netdev(adapter->pnetdev);
		adapter->pnetdev = NULL;
	}
}

/* For ethtool +++ */
#ifdef CONFIG_IOCTL_CFG80211
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 8))
static void rtw_ethtool_get_drvinfo(struct net_device *dev, struct ethtool_drvinfo *info)
{
	struct wireless_dev *wdev = NULL;
	_adapter *padapter = NULL;
	HAL_DATA_TYPE *hal_data = NULL;

	wdev = dev->ieee80211_ptr;
	if (wdev) {
		strlcpy(info->driver, wiphy_dev(wdev->wiphy)->driver->name,
			sizeof(info->driver));
	} else {
		strlcpy(info->driver, "N/A", sizeof(info->driver));
	}

	strlcpy(info->version, DRIVERVERSION, sizeof(info->version));

	padapter = (_adapter *)rtw_netdev_priv(dev);
	if (padapter) {
		hal_data = GET_HAL_DATA(padapter);
	}

	if (hal_data) {
		scnprintf(info->fw_version, sizeof(info->fw_version), "%d.%d",
			  hal_data->firmware_version, hal_data->firmware_sub_version);
	} else {
		strlcpy(info->fw_version, "N/A", sizeof(info->fw_version));
	}

	strlcpy(info->bus_info, dev_name(wiphy_dev(wdev->wiphy)),
		sizeof(info->bus_info));
}

static const char rtw_ethtool_gstrings_sta_stats[][ETH_GSTRING_LEN] = {
	"rx_packets", "rx_bytes", "rx_dropped",
	"tx_packets", "tx_bytes", "tx_dropped",
};

#define RTW_ETHTOOL_STATS_LEN	ARRAY_SIZE(rtw_ethtool_gstrings_sta_stats)

static int rtw_ethtool_get_sset_count(struct net_device *dev, int sset)
{
	int rv = 0;

	if (sset == ETH_SS_STATS)
		rv += RTW_ETHTOOL_STATS_LEN;

	if (rv == 0)
		return -EOPNOTSUPP;

	return rv;
}

static void rtw_ethtool_get_strings(struct net_device *dev, u32 sset, u8 *data)
{
	int sz_sta_stats = 0;

	if (sset == ETH_SS_STATS) {
		sz_sta_stats = sizeof(rtw_ethtool_gstrings_sta_stats);
		memcpy(data, rtw_ethtool_gstrings_sta_stats, sz_sta_stats);
	}
}

static void rtw_ethtool_get_stats(struct net_device *dev,
				  struct ethtool_stats *stats,
				  u64 *data)
{
	int i = 0;
	_adapter *padapter = NULL;
	struct xmit_priv *pxmitpriv = NULL;
	struct recv_priv *precvpriv = NULL;

	memset(data, 0, sizeof(u64) * RTW_ETHTOOL_STATS_LEN);
	
	padapter = (_adapter *)rtw_netdev_priv(dev);
	if (padapter) {
		pxmitpriv = &(padapter->xmitpriv);
		precvpriv = &adapter_to_dvobj(padapter)->recvpriv;

		data[i++] = precvpriv->rx_pkts;
		data[i++] = precvpriv->rx_bytes;
		data[i++] = precvpriv->rx_drop;

		data[i++] = pxmitpriv->tx_pkts;
		data[i++] = pxmitpriv->tx_bytes;
		data[i++] = pxmitpriv->tx_drop;
	} else {
		data[i++] = 0;
		data[i++] = 0;
		data[i++] = 0;

		data[i++] = 0;
		data[i++] = 0;
		data[i++] = 0;
	}
}

static const struct ethtool_ops rtw_ethtool_ops = {
	.get_drvinfo = rtw_ethtool_get_drvinfo,
	.get_link = ethtool_op_get_link,
	.get_strings = rtw_ethtool_get_strings,
	.get_ethtool_stats = rtw_ethtool_get_stats,
	.get_sset_count = rtw_ethtool_get_sset_count,
};
#endif // LINUX_VERSION_CODE >= 3.7.8 
#endif /* CONFIG_IOCTL_CFG80211 */
/* For ethtool --- */

int rtw_os_ndev_register(_adapter *adapter, const char *name)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	int ret = _SUCCESS;
	struct net_device *ndev = adapter->pnetdev;
	u8 rtnl_lock_needed = rtw_rtnl_lock_needed(dvobj);

#ifdef CONFIG_RTW_NAPI
	netif_napi_add(ndev, &adapter->napi, rtw_recv_napi_poll, RTL_NAPI_WEIGHT);
#endif /* CONFIG_RTW_NAPI */

#if defined(CONFIG_IOCTL_CFG80211)
	if (rtw_cfg80211_ndev_res_register(adapter) != _SUCCESS) {
		rtw_warn_on(1);
		ret = _FAIL;
		goto exit;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 8))
	netdev_set_default_ethtool_ops(ndev, &rtw_ethtool_ops);
#endif /* LINUX_VERSION_CODE >= 3.7.8 */
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0)) && defined(CONFIG_PCI_HCI)
	ndev->gro_flush_timeout = 100000;
#endif
	/* alloc netdev name */
	rtw_init_netdev_name(ndev, name);

	_rtw_memcpy(ndev->dev_addr, adapter_mac_addr(adapter), ETH_ALEN);

	/* Tell the network stack we exist */

	if (rtnl_lock_needed)
		ret = (register_netdev(ndev) == 0) ? _SUCCESS : _FAIL;
	else
		ret = (register_netdevice(ndev) == 0) ? _SUCCESS : _FAIL;

	if (ret == _SUCCESS)
		adapter->registered = 1;
	else
		RTW_INFO(FUNC_NDEV_FMT" if%d Failed!\n", FUNC_NDEV_ARG(ndev), (adapter->iface_id + 1));

#if defined(CONFIG_IOCTL_CFG80211)
	if (ret != _SUCCESS) {
		rtw_cfg80211_ndev_res_unregister(adapter);
		#if !defined(RTW_SINGLE_WIPHY)
		rtw_wiphy_unregister(adapter_to_wiphy(adapter));
		#endif
	}
#endif

#if defined(CONFIG_IOCTL_CFG80211)
exit:
#endif
#ifdef CONFIG_RTW_NAPI
	if (ret != _SUCCESS)
		netif_napi_del(&adapter->napi);
#endif /* CONFIG_RTW_NAPI */

	return ret;
}

void rtw_os_ndev_unregister(_adapter *adapter)
{
	struct net_device *netdev = NULL;

	if (adapter == NULL || adapter->registered == 0)
		return;

	adapter->ndev_unregistering = 1;

	netdev = adapter->pnetdev;

#if defined(CONFIG_IOCTL_CFG80211)
	rtw_cfg80211_ndev_res_unregister(adapter);
#endif

	if ((adapter->DriverState != DRIVER_DISAPPEAR) && netdev) {
		struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
		u8 rtnl_lock_needed = rtw_rtnl_lock_needed(dvobj);

		if (rtnl_lock_needed)
			unregister_netdev(netdev);
		else
			unregister_netdevice(netdev);
	}

#if defined(CONFIG_IOCTL_CFG80211) && !defined(RTW_SINGLE_WIPHY)
#ifdef CONFIG_RFKILL_POLL
	rtw_cfg80211_deinit_rfkill(adapter_to_wiphy(adapter));
#endif
	rtw_wiphy_unregister(adapter_to_wiphy(adapter));
#endif

#ifdef CONFIG_RTW_NAPI
	if (adapter->napi_state == NAPI_ENABLE) {
		napi_disable(&adapter->napi);
		adapter->napi_state = NAPI_DISABLE;
	}
	netif_napi_del(&adapter->napi);
#endif /* CONFIG_RTW_NAPI */

	adapter->registered = 0;
	adapter->ndev_unregistering = 0;
}

/**
 * rtw_os_ndev_init - Allocate and register OS layer net device and relating structures for @adapter
 * @adapter: the adapter on which this function applies
 * @name: the requesting net device name
 *
 * Returns:
 * _SUCCESS or _FAIL
 */
int rtw_os_ndev_init(_adapter *adapter, const char *name)
{
	int ret = _FAIL;

	if (rtw_os_ndev_alloc(adapter) != _SUCCESS)
		goto exit;

	if (rtw_os_ndev_register(adapter, name) != _SUCCESS)
		goto os_ndev_free;

	ret = _SUCCESS;

os_ndev_free:
	if (ret != _SUCCESS)
		rtw_os_ndev_free(adapter);
exit:
	return ret;
}

/**
 * rtw_os_ndev_deinit - Unregister and free OS layer net device and relating structures for @adapter
 * @adapter: the adapter on which this function applies
 */
void rtw_os_ndev_deinit(_adapter *adapter)
{
	rtw_os_ndev_unregister(adapter);
	rtw_os_ndev_free(adapter);
}

int rtw_os_ndevs_alloc(struct dvobj_priv *dvobj)
{
	int i, status = _SUCCESS;
	_adapter *adapter;

#if defined(CONFIG_IOCTL_CFG80211)
	if (rtw_cfg80211_dev_res_alloc(dvobj) != _SUCCESS) {
		rtw_warn_on(1);
		return _FAIL;
	}
#endif

	for (i = 0; i < dvobj->iface_nums; i++) {

		if (i >= CONFIG_IFACE_NUMBER) {
			RTW_ERR("%s %d >= CONFIG_IFACE_NUMBER(%d)\n", __func__, i, CONFIG_IFACE_NUMBER);
			rtw_warn_on(1);
			continue;
		}

		adapter = dvobj->padapters[i];
		if (adapter && !adapter->pnetdev) {

			#ifdef CONFIG_RTW_DYNAMIC_NDEV
			if (!is_primary_adapter(adapter))
				continue;
			#endif

			status = rtw_os_ndev_alloc(adapter);
			if (status != _SUCCESS) {
				rtw_warn_on(1);
				break;
			}
		}
	}

	if (status != _SUCCESS) {
		for (; i >= 0; i--) {
			adapter = dvobj->padapters[i];
			if (adapter && adapter->pnetdev)
				rtw_os_ndev_free(adapter);
		}
	}

#if defined(CONFIG_IOCTL_CFG80211)
	if (status != _SUCCESS)
		rtw_cfg80211_dev_res_free(dvobj);
#endif

	return status;
}

void rtw_os_ndevs_free(struct dvobj_priv *dvobj)
{
	int i;
	_adapter *adapter = NULL;

	for (i = 0; i < dvobj->iface_nums; i++) {

		if (i >= CONFIG_IFACE_NUMBER) {
			RTW_ERR("%s %d >= CONFIG_IFACE_NUMBER(%d)\n", __func__, i, CONFIG_IFACE_NUMBER);
			rtw_warn_on(1);
			continue;
		}

		adapter = dvobj->padapters[i];

		if (adapter == NULL)
			continue;

		rtw_os_ndev_free(adapter);
	}

#if defined(CONFIG_IOCTL_CFG80211)
	rtw_cfg80211_dev_res_free(dvobj);
#endif
}

u32 rtw_start_drv_threads(_adapter *padapter)
{
	u32 _status = _SUCCESS;

	RTW_INFO(FUNC_ADPT_FMT" enter\n", FUNC_ADPT_ARG(padapter));

#ifdef CONFIG_XMIT_THREAD_MODE
#if defined(CONFIG_SDIO_HCI)
	if (is_primary_adapter(padapter))
#endif
	{
		if (padapter->xmitThread == NULL) {
			RTW_INFO(FUNC_ADPT_FMT " start RTW_XMIT_THREAD\n", FUNC_ADPT_ARG(padapter));
			padapter->xmitThread = kthread_run(rtw_xmit_thread, padapter, "RTW_XMIT_THREAD");
			if (IS_ERR(padapter->xmitThread)) {
				padapter->xmitThread = NULL;
				_status = _FAIL;
			}
		}
	}
#endif /* #ifdef CONFIG_XMIT_THREAD_MODE */

#ifdef CONFIG_RECV_THREAD_MODE
	if (is_primary_adapter(padapter)) {
		if (padapter->recvThread == NULL) {
			RTW_INFO(FUNC_ADPT_FMT " start RTW_RECV_THREAD\n", FUNC_ADPT_ARG(padapter));
			padapter->recvThread = kthread_run(rtw_recv_thread, padapter, "RTW_RECV_THREAD");
			if (IS_ERR(padapter->recvThread)) {
				padapter->recvThread = NULL;
				_status = _FAIL;
			}
		}
	}
#endif

	if (is_primary_adapter(padapter)) {
		if (padapter->cmdThread == NULL) {
			RTW_INFO(FUNC_ADPT_FMT " start RTW_CMD_THREAD\n", FUNC_ADPT_ARG(padapter));
			padapter->cmdThread = kthread_run(rtw_cmd_thread, padapter, "RTW_CMD_THREAD");
			if (IS_ERR(padapter->cmdThread)) {
				padapter->cmdThread = NULL;
				_status = _FAIL;
			}
			else
				_rtw_down_sema(&adapter_to_dvobj(padapter)->cmdpriv.start_cmdthread_sema); /* wait for cmd_thread to run */
		}
	}


#ifdef CONFIG_EVENT_THREAD_MODE
	if (padapter->evtThread == NULL) {
		RTW_INFO(FUNC_ADPT_FMT " start RTW_EVENT_THREAD\n", FUNC_ADPT_ARG(padapter));
		padapter->evtThread = kthread_run(event_thread, padapter, "RTW_EVENT_THREAD");
		if (IS_ERR(padapter->evtThread)) {
			padapter->evtThread = NULL;
			_status = _FAIL;
		}
	}
#endif

	rtw_hal_start_thread(padapter);
	return _status;

}

void rtw_stop_drv_threads(_adapter *padapter)
{
	RTW_INFO(FUNC_ADPT_FMT" enter\n", FUNC_ADPT_ARG(padapter));
	if (is_primary_adapter(padapter))
		rtw_stop_cmd_thread(padapter);

#ifdef CONFIG_EVENT_THREAD_MODE
	if (padapter->evtThread) {
		_rtw_up_sema(&padapter->evtpriv.evt_notify);
		rtw_thread_stop(padapter->evtThread);
		padapter->evtThread = NULL;
	}
#endif

#ifdef CONFIG_XMIT_THREAD_MODE
	/* Below is to termindate tx_thread... */
#if defined(CONFIG_SDIO_HCI)
	/* Only wake-up primary adapter */
	if (is_primary_adapter(padapter))
#endif  /*SDIO_HCI */
	{
		if (padapter->xmitThread) {
			_rtw_up_sema(&padapter->xmitpriv.xmit_sema);
			rtw_thread_stop(padapter->xmitThread);
			padapter->xmitThread = NULL;
		}
	}
#endif

#ifdef CONFIG_RECV_THREAD_MODE
	if (is_primary_adapter(padapter) && padapter->recvThread) {
		/* Below is to termindate rx_thread... */
		_rtw_up_sema(&adapter_to_dvobj(padapter)->recvpriv.recv_sema);
		rtw_thread_stop(padapter->recvThread);
		padapter->recvThread = NULL;
	}
#endif

	rtw_hal_stop_thread(padapter);
}

u8 rtw_init_default_value(_adapter *padapter)
{
	u8 ret  = _SUCCESS;
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;

	/* xmit_priv */
	pxmitpriv->vcs_setting = pregistrypriv->vrtl_carrier_sense;
	pxmitpriv->vcs = pregistrypriv->vcs_type;
	pxmitpriv->vcs_type = pregistrypriv->vcs_type;
	/* pxmitpriv->rts_thresh = pregistrypriv->rts_thresh; */
	pxmitpriv->frag_len = pregistrypriv->frag_thresh;

	/* security_priv */
	/* rtw_get_encrypt_decrypt_from_registrypriv(padapter); */
	psecuritypriv->binstallGrpkey = _FAIL;
#ifdef CONFIG_GTK_OL
	psecuritypriv->binstallKCK_KEK = _FAIL;
#endif /* CONFIG_GTK_OL */
	psecuritypriv->sw_encrypt = pregistrypriv->software_encrypt;
	psecuritypriv->sw_decrypt = pregistrypriv->software_decrypt;

	psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Open; /* open system */
	psecuritypriv->dot11PrivacyAlgrthm = _NO_PRIVACY_;

	psecuritypriv->dot11PrivacyKeyIndex = 0;

	psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
	psecuritypriv->dot118021XGrpKeyid = 1;

	psecuritypriv->ndisauthtype = Ndis802_11AuthModeOpen;
	psecuritypriv->ndisencryptstatus = Ndis802_11WEPDisabled;
	psecuritypriv->dot118021x_bmc_cam_id = INVALID_SEC_MAC_CAM_ID;


	/* pwrctrl_priv */


	/* registry_priv */
	rtw_init_registrypriv_dev_network(padapter);
	rtw_update_registrypriv_dev_network(padapter);


	/* hal_priv */
	rtw_hal_def_value_init(padapter);

#ifdef CONFIG_MCC_MODE
	/* MCC parameter */
	rtw_hal_mcc_parameter_init(padapter);
#endif /* CONFIG_MCC_MODE */

	/* misc. */
	RTW_ENABLE_FUNC(adapter_to_dvobj(padapter), DF_RX_BIT);
	RTW_ENABLE_FUNC(adapter_to_dvobj(padapter), DF_TX_BIT);
	padapter->bLinkInfoDump = 0;
	padapter->bNotifyChannelChange = _FALSE;
#ifdef CONFIG_P2P
	padapter->bShowGetP2PState = 1;
#endif

	/* for debug purpose */
	padapter->fix_rate = 0xFF;
	padapter->data_fb = 0;
	padapter->fix_bw = 0xFF;
	padapter->power_offset = 0;
	padapter->rsvd_page_offset = 0;
	padapter->rsvd_page_num = 0;
#ifdef CONFIG_AP_MODE
	padapter->bmc_tx_rate = pregistrypriv->bmc_tx_rate;
	#if CONFIG_RTW_AP_DATA_BMC_TO_UC
	padapter->b2u_flags_ap_src = pregistrypriv->ap_src_b2u_flags;
	padapter->b2u_flags_ap_fwd = pregistrypriv->ap_fwd_b2u_flags;
	#endif
#endif
	padapter->driver_tx_bw_mode = pregistrypriv->tx_bw_mode;

	padapter->driver_ampdu_spacing = 0xFF;
	padapter->driver_rx_ampdu_factor =  0xFF;
	padapter->driver_rx_ampdu_spacing = 0xFF;
	padapter->fix_rx_ampdu_accept = RX_AMPDU_ACCEPT_INVALID;
	padapter->fix_rx_ampdu_size = RX_AMPDU_SIZE_INVALID;
#ifdef CONFIG_TX_AMSDU
	padapter->tx_amsdu = 2;
	padapter->tx_amsdu_rate = 400;
#endif
	padapter->driver_tx_max_agg_num = 0xFF;
#ifdef DBG_RX_COUNTER_DUMP
	padapter->dump_rx_cnt_mode = 0;
	padapter->drv_rx_cnt_ok = 0;
	padapter->drv_rx_cnt_crcerror = 0;
	padapter->drv_rx_cnt_drop = 0;
#endif
#ifdef CONFIG_RTW_NAPI
	padapter->napi_state = NAPI_DISABLE;
#endif

#ifdef CONFIG_RTW_ACS
	if (pregistrypriv->acs_mode)
		rtw_acs_start(padapter);
	else
		rtw_acs_stop(padapter);
#endif
#ifdef CONFIG_BACKGROUND_NOISE_MONITOR
	if (pregistrypriv->nm_mode)
		rtw_nm_enable(padapter);
	else
		rtw_nm_disable(padapter);
#endif

#ifdef CONFIG_RTW_TOKEN_BASED_XMIT
	ATOMIC_SET(&padapter->tbtx_tx_pause, _FALSE);
	ATOMIC_SET(&padapter->tbtx_remove_tx_pause, _FALSE);
	padapter->tbtx_capability = _TRUE;
#endif

	return ret;
}
#ifdef CONFIG_CLIENT_PORT_CFG
extern void rtw_clt_port_init(struct clt_port_t  *cltp);
extern void rtw_clt_port_deinit(struct clt_port_t  *cltp);
#endif

struct dvobj_priv *devobj_init(void)
{
	struct dvobj_priv *pdvobj = NULL;

	pdvobj = (struct dvobj_priv *)rtw_zmalloc(sizeof(*pdvobj));
	if (pdvobj == NULL)
		return NULL;

	_rtw_mutex_init(&pdvobj->hw_init_mutex);
	_rtw_mutex_init(&pdvobj->h2c_fwcmd_mutex);
	_rtw_mutex_init(&pdvobj->setch_mutex);
	_rtw_mutex_init(&pdvobj->setbw_mutex);
	_rtw_mutex_init(&pdvobj->rf_read_reg_mutex);
	_rtw_mutex_init(&pdvobj->ioctrl_mutex);
#ifdef CONFIG_SDIO_INDIRECT_ACCESS
	_rtw_mutex_init(&pdvobj->sd_indirect_access_mutex);
#endif
#ifdef CONFIG_SYSON_INDIRECT_ACCESS
	_rtw_mutex_init(&pdvobj->syson_indirect_access_mutex);
#endif
#ifdef CONFIG_RTW_CUSTOMER_STR
	_rtw_mutex_init(&pdvobj->customer_str_mutex);
	_rtw_memset(pdvobj->customer_str, 0xFF, RTW_CUSTOMER_STR_LEN);
#endif
#ifdef CONFIG_PROTSEL_PORT
	_rtw_mutex_init(&pdvobj->protsel_port.mutex);
#endif
#ifdef CONFIG_PROTSEL_ATIMDTIM
	_rtw_mutex_init(&pdvobj->protsel_atimdtim.mutex);
#endif
#ifdef CONFIG_PROTSEL_MACSLEEP
	_rtw_mutex_init(&pdvobj->protsel_macsleep.mutex);
#endif

	pdvobj->processing_dev_remove = _FALSE;

	ATOMIC_SET(&pdvobj->disable_func, 0);

	rtw_macid_ctl_init(&pdvobj->macid_ctl);
#ifdef CONFIG_CLIENT_PORT_CFG
	rtw_clt_port_init(&pdvobj->clt_port);
#endif
	_rtw_spinlock_init(&pdvobj->cam_ctl.lock);
	_rtw_mutex_init(&pdvobj->cam_ctl.sec_cam_access_mutex);
#if defined(CONFIG_PLATFORM_RTK129X) && defined(CONFIG_PCI_HCI)
	_rtw_spinlock_init(&pdvobj->io_reg_lock);
#endif
#ifdef CONFIG_MBSSID_CAM
	rtw_mbid_cam_init(pdvobj);
#endif

#ifdef CONFIG_AP_MODE
	#ifdef CONFIG_SUPPORT_MULTI_BCN
	pdvobj->nr_ap_if = 0;
	pdvobj->inter_bcn_space = DEFAULT_BCN_INTERVAL; /* default value is equal to the default beacon_interval (100ms) */
	_rtw_init_queue(&pdvobj->ap_if_q);
	pdvobj->vap_map = 0;
	#endif /*CONFIG_SUPPORT_MULTI_BCN*/
	#ifdef CONFIG_SWTIMER_BASED_TXBCN
	rtw_init_timer(&(pdvobj->txbcn_timer), tx_beacon_timer_handlder, pdvobj);
	#endif
#endif

	rtw_init_timer(&(pdvobj->dynamic_chk_timer), rtw_dynamic_check_timer_handlder, pdvobj);
	rtw_init_timer(&(pdvobj->periodic_tsf_update_end_timer), rtw_hal_periodic_tsf_update_end_timer_hdl, pdvobj);

#ifdef CONFIG_MCC_MODE
	_rtw_mutex_init(&(pdvobj->mcc_objpriv.mcc_mutex));
	_rtw_mutex_init(&(pdvobj->mcc_objpriv.mcc_tsf_req_mutex));
	_rtw_mutex_init(&(pdvobj->mcc_objpriv.mcc_dbg_reg_mutex));
	_rtw_spinlock_init(&pdvobj->mcc_objpriv.mcc_lock);
#endif /* CONFIG_MCC_MODE */

#ifdef CONFIG_RTW_NAPI_DYNAMIC
	pdvobj->en_napi_dynamic = 0;
#endif /* CONFIG_RTW_NAPI_DYNAMIC */


#ifdef CONFIG_RTW_TPT_MODE
	pdvobj->tpt_mode = 0;
	pdvobj->edca_be_ul = 0x5ea42b;
	pdvobj->edca_be_dl = 0x00a42b;
#endif 
	pdvobj->scan_deny = _FALSE;

	/* wpas type default from w1.fi */
	pdvobj->wpas_type = RTW_WPAS_W1FI;

	return pdvobj;

}

void devobj_deinit(struct dvobj_priv *pdvobj)
{
	if (!pdvobj)
		return;

	/* TODO: use rtw_os_ndevs_deinit instead at the first stage of driver's dev deinit function */
#if defined(CONFIG_IOCTL_CFG80211)
	rtw_cfg80211_dev_res_free(pdvobj);
#endif

#ifdef CONFIG_MCC_MODE
	_rtw_mutex_free(&(pdvobj->mcc_objpriv.mcc_mutex));
	_rtw_mutex_free(&(pdvobj->mcc_objpriv.mcc_tsf_req_mutex));
	_rtw_mutex_free(&(pdvobj->mcc_objpriv.mcc_dbg_reg_mutex));
	_rtw_spinlock_free(&pdvobj->mcc_objpriv.mcc_lock);
#endif /* CONFIG_MCC_MODE */

	_rtw_mutex_free(&pdvobj->hw_init_mutex);
	_rtw_mutex_free(&pdvobj->h2c_fwcmd_mutex);

#ifdef CONFIG_RTW_CUSTOMER_STR
	_rtw_mutex_free(&pdvobj->customer_str_mutex);
#endif
#ifdef CONFIG_PROTSEL_PORT
	_rtw_mutex_free(&pdvobj->protsel_port.mutex);
#endif
#ifdef CONFIG_PROTSEL_ATIMDTIM
	_rtw_mutex_free(&pdvobj->protsel_atimdtim.mutex);
#endif
#ifdef CONFIG_PROTSEL_MACSLEEP
	_rtw_mutex_free(&pdvobj->protsel_macsleep.mutex);
#endif

	_rtw_mutex_free(&pdvobj->setch_mutex);
	_rtw_mutex_free(&pdvobj->setbw_mutex);
	_rtw_mutex_free(&pdvobj->rf_read_reg_mutex);
	_rtw_mutex_free(&pdvobj->ioctrl_mutex);
#ifdef CONFIG_SDIO_INDIRECT_ACCESS
	_rtw_mutex_free(&pdvobj->sd_indirect_access_mutex);
#endif
#ifdef CONFIG_SYSON_INDIRECT_ACCESS
	_rtw_mutex_free(&pdvobj->syson_indirect_access_mutex);
#endif

	rtw_macid_ctl_deinit(&pdvobj->macid_ctl);
#ifdef CONFIG_CLIENT_PORT_CFG
	rtw_clt_port_deinit(&pdvobj->clt_port);
#endif

	_rtw_spinlock_free(&pdvobj->cam_ctl.lock);
	_rtw_mutex_free(&pdvobj->cam_ctl.sec_cam_access_mutex);

#if defined(CONFIG_PLATFORM_RTK129X) && defined(CONFIG_PCI_HCI)
	_rtw_spinlock_free(&pdvobj->io_reg_lock);
#endif
#ifdef CONFIG_MBSSID_CAM
	rtw_mbid_cam_deinit(pdvobj);
#endif
#ifdef CONFIG_SUPPORT_MULTI_BCN
	_rtw_spinlock_free(&(pdvobj->ap_if_q.lock));
#endif
	rtw_mfree((u8 *)pdvobj, sizeof(*pdvobj));
}

inline u8 rtw_rtnl_lock_needed(struct dvobj_priv *dvobj)
{
	if (dvobj->rtnl_lock_holder && dvobj->rtnl_lock_holder == current)
		return 0;
	return 1;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26))
static inline int rtnl_is_locked(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 17))
	if (unlikely(rtnl_trylock())) {
		rtnl_unlock();
#else
	if (unlikely(down_trylock(&rtnl_sem) == 0)) {
		up(&rtnl_sem);
#endif
		return 0;
	}
	return 1;
}
#endif

inline void rtw_set_rtnl_lock_holder(struct dvobj_priv *dvobj, _thread_hdl_ thd_hdl)
{
	rtw_warn_on(!rtnl_is_locked());

	if (!thd_hdl || rtnl_is_locked())
		dvobj->rtnl_lock_holder = thd_hdl;

	if (dvobj->rtnl_lock_holder && 0)
		RTW_INFO("rtnl_lock_holder: %s:%d\n", current->comm, current->pid);
}

u8 rtw_reset_drv_sw(_adapter *padapter)
{
	u8	ret8 = _SUCCESS;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);

	/* hal_priv */
	rtw_hal_def_value_init(padapter);

	RTW_ENABLE_FUNC(adapter_to_dvobj(padapter), DF_RX_BIT);
	RTW_ENABLE_FUNC(adapter_to_dvobj(padapter), DF_TX_BIT);

	padapter->bLinkInfoDump = 0;

	padapter->xmitpriv.tx_pkts = 0;
	adapter_to_dvobj(padapter)->recvpriv.rx_pkts = 0;

	pmlmepriv->LinkDetectInfo.bBusyTraffic = _FALSE;

	/* pmlmepriv->LinkDetectInfo.TrafficBusyState = _FALSE; */
	pmlmepriv->LinkDetectInfo.TrafficTransitionCount = 0;
	pmlmepriv->LinkDetectInfo.LowPowerTransitionCount = 0;

	_clr_fwstate_(pmlmepriv, WIFI_UNDER_SURVEY | WIFI_UNDER_LINKING);

#ifdef DBG_CONFIG_ERROR_DETECT
	if (is_primary_adapter(padapter))
		rtw_hal_sreset_reset_value(padapter);
#endif
	pwrctrlpriv->pwr_state_check_cnts = 0;

	/* mlmeextpriv */
	mlmeext_set_scan_state(&padapter->mlmeextpriv, SCAN_DISABLE);

#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
	rtw_set_signal_stat_timer(&adapter_to_dvobj(padapter)->recvpriv);
#endif

	return ret8;
}

u8 devobj_trx_resource_init(struct dvobj_priv *dvobj)
{
	u8 ret = _SUCCESS;

#ifdef CONFIG_USB_HCI
	ret = rtw_init_lite_xmit_resource(dvobj);
	if (ret == _FAIL)
		goto exit;
	ret = rtw_init_lite_recv_resource(dvobj);
	if (ret == _FAIL)
		goto exit;
#endif

	ret = rtw_init_cmd_priv(dvobj);
	if (ret == _FAIL) {
		RTW_ERR("%s rtw_init_cmd_priv failed\n", __func__);
		goto exit;
	}

exit:
	return ret;
}

void devobj_trx_resource_deinit(struct dvobj_priv *dvobj)
{
#ifdef CONFIG_USB_HCI
	rtw_free_lite_xmit_resource(dvobj);
	rtw_free_lite_recv_resource(dvobj);
#endif
	rtw_free_cmd_priv(dvobj);
}


u8 rtw_init_drv_sw(_adapter *padapter)
{
	u8	ret8 = _SUCCESS;

#ifdef CONFIG_RTW_CFGVENDOR_RANDOM_MAC_OUI
	struct rtw_wdev_priv *pwdev_priv = adapter_wdev_data(padapter);
#endif

	#if defined(CONFIG_AP_MODE) && defined(CONFIG_SUPPORT_MULTI_BCN)
	_rtw_init_listhead(&padapter->list);
	#ifdef CONFIG_FW_HANDLE_TXBCN
	padapter->vap_id = CONFIG_LIMITED_AP_NUM;
	if (is_primary_adapter(padapter))
		adapter_to_dvobj(padapter)->vap_tbtt_rpt_map = adapter_to_regsty(padapter)->fw_tbtt_rpt;
	#endif
	#endif

	#ifdef CONFIG_CLIENT_PORT_CFG
	padapter->client_id = MAX_CLIENT_PORT_NUM;
	padapter->client_port = CLT_PORT_INVALID;
	#endif

	if (is_primary_adapter(padapter)) {
		struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
		struct hal_spec_t *hal_spec = GET_HAL_SPEC(padapter);

		dvobj->macid_ctl.num = rtw_min(hal_spec->macid_num, MACID_NUM_SW_LIMIT);
		dvobj->macid_ctl.macid_cap = hal_spec->macid_cap;
		dvobj->macid_ctl.macid_txrpt = hal_spec->macid_txrpt;
		dvobj->macid_ctl.macid_txrpt_pgsz = hal_spec->macid_txrpt_pgsz;
		dvobj->cam_ctl.sec_cap = hal_spec->sec_cap;
		dvobj->cam_ctl.num = rtw_min(hal_spec->sec_cam_ent_num, SEC_CAM_ENT_NUM_SW_LIMIT);
		
		dvobj->wow_ctl.wow_cap = hal_spec->wow_cap;

		#ifdef CONFIG_SDIO_TX_ENABLE_AVAL_INT
		dvobj->tx_aval_int_thr_mode = 2; /*setting by max tx length*/
		dvobj->tx_aval_int_thr_value = 0;
		#endif /*CONFIG_SDIO_TX_ENABLE_AVAL_INT*/
		
		#if CONFIG_TX_AC_LIFETIME
		{
			struct registry_priv *regsty = adapter_to_regsty(padapter);
			int i;

			dvobj->tx_aclt_flags = regsty->tx_aclt_flags;
			for (i = 0; i < TX_ACLT_CONF_NUM; i++) {
				dvobj->tx_aclt_confs[i].en = regsty->tx_aclt_confs[i].en;
				dvobj->tx_aclt_confs[i].vo_vi
					= regsty->tx_aclt_confs[i].vo_vi / (hal_spec->tx_aclt_unit_factor * 32);
				if (dvobj->tx_aclt_confs[i].vo_vi > 0xFFFF)
					dvobj->tx_aclt_confs[i].vo_vi = 0xFFFF;
				dvobj->tx_aclt_confs[i].be_bk
					= regsty->tx_aclt_confs[i].be_bk / (hal_spec->tx_aclt_unit_factor * 32);
				if (dvobj->tx_aclt_confs[i].be_bk > 0xFFFF)
					dvobj->tx_aclt_confs[i].be_bk = 0xFFFF;
			}

			dvobj->tx_aclt_force_val.en = 0xFF;
		}
		#endif
		#if defined (CONFIG_CONCURRENT_MODE)  && defined (CONFIG_TSF_SYNC)
		dvobj->sync_tsfr_counter = 0x0;
		#endif
	}

	ret8 = rtw_init_default_value(padapter);

#if 0 // NEO : move to devobj_trx_resource_init
	if ((rtw_init_cmd_priv(&padapter->cmdpriv)) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}
#endif // if 0 NEO

	adapter_to_dvobj(padapter)->cmdpriv.padapter = padapter;

	if ((rtw_init_evt_priv(&padapter->evtpriv)) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}

	if (is_primary_adapter(padapter)) {
		if (rtw_hal_rfpath_init(padapter) == _FAIL) {
			ret8 = _FAIL;
			goto exit;
		}
		if (rtw_hal_trxnss_init(padapter) == _FAIL) {
			ret8 = _FAIL;
			goto exit;
		}
		if (rtw_hal_runtime_trx_path_decision(padapter) == _FAIL) {
			ret8 = _FAIL;
			goto exit;
		}
		if (rtw_rfctl_init(padapter) == _FAIL) {
			ret8 = _FAIL;
			goto exit;
		}
	}

	if (rtw_init_mlme_priv(padapter) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}

#if (defined(CONFIG_P2P) && defined(CONFIG_CONCURRENT_MODE)) || defined(CONFIG_IOCTL_CFG80211)
	rtw_init_roch_info(padapter);
#endif

#ifdef CONFIG_P2P
	rtw_init_wifidirect_timers(padapter);
	init_wifidirect_info(padapter, P2P_ROLE_DISABLE);
	reset_global_wifidirect_info(padapter);
#ifdef CONFIG_WFD
	if (rtw_init_wifi_display_info(padapter) == _FAIL)
		RTW_ERR("Can't init init_wifi_display_info\n");
#endif
#endif /* CONFIG_P2P */

	if (init_mlme_ext_priv(padapter) == _FAIL) {
		ret8 = _FAIL;
		goto exit;
	}

#ifdef CONFIG_TDLS
	if (rtw_init_tdls_info(padapter) == _FAIL) {
		RTW_INFO("Can't rtw_init_tdls_info\n");
		ret8 = _FAIL;
		goto exit;
	}
#endif /* CONFIG_TDLS */

#ifdef CONFIG_RTW_MESH
	rtw_mesh_cfg_init(padapter);
#endif

	if (_rtw_init_xmit_priv(&padapter->xmitpriv, padapter) == _FAIL) {
		RTW_INFO("Can't _rtw_init_xmit_priv\n");
		ret8 = _FAIL;
		goto exit;
	}

	if (rtw_init_recv_priv(&adapter_to_dvobj(padapter)->recvpriv, padapter) == _FAIL) {
		RTW_INFO("Can't rtw_init_recv_priv\n");
		ret8 = _FAIL;
		goto exit;
	}
	/* add for CONFIG_IEEE80211W, none 11w also can use */
	_rtw_spinlock_init(&padapter->security_key_mutex);

	/* We don't need to memset padapter->XXX to zero, because adapter is allocated by rtw_zvmalloc(). */
	/* _rtw_memset((unsigned char *)&padapter->securitypriv, 0, sizeof (struct security_priv)); */

	if (_rtw_init_sta_priv(&padapter->stapriv) == _FAIL) {
		RTW_INFO("Can't _rtw_init_sta_priv\n");
		ret8 = _FAIL;
		goto exit;
	}

	padapter->setband = WIFI_FREQUENCY_BAND_AUTO;
	padapter->fix_rate = 0xFF;
	padapter->power_offset = 0;
	padapter->rsvd_page_offset = 0;
	padapter->rsvd_page_num = 0;

	padapter->data_fb = 0;
	padapter->fix_rx_ampdu_accept = RX_AMPDU_ACCEPT_INVALID;
	padapter->fix_rx_ampdu_size = RX_AMPDU_SIZE_INVALID;
#ifdef DBG_RX_COUNTER_DUMP
	padapter->dump_rx_cnt_mode = 0;
	padapter->drv_rx_cnt_ok = 0;
	padapter->drv_rx_cnt_crcerror = 0;
	padapter->drv_rx_cnt_drop = 0;
#endif
	rtw_init_bcmc_stainfo(padapter);

	rtw_init_pwrctrl_priv(padapter);

	/* _rtw_memset((u8 *)&padapter->qospriv, 0, sizeof (struct qos_priv)); */ /* move to mlme_priv */

#ifdef CONFIG_MP_INCLUDED
	if (init_mp_priv(padapter) == _FAIL)
		RTW_INFO("%s: initialize MP private data Fail!\n", __func__);
#endif

	rtw_hal_dm_init(padapter);

	if (is_primary_adapter(padapter))
		rtw_rfctl_chplan_init(padapter);

#ifdef CONFIG_RTW_SW_LED
	rtw_hal_sw_led_init(padapter);
#endif
#ifdef DBG_CONFIG_ERROR_DETECT
	rtw_hal_sreset_init(padapter);
#endif

#ifdef CONFIG_WAPI_SUPPORT
	padapter->WapiSupport = true; /* set true temp, will revise according to Efuse or Registry value later. */
	rtw_wapi_init(padapter);
#endif

#ifdef CONFIG_BR_EXT
	_rtw_spinlock_init(&padapter->br_ext_lock);
#endif /* CONFIG_BR_EXT */

#ifdef CONFIG_BEAMFORMING
#ifdef RTW_BEAMFORMING_VERSION_2
	rtw_bf_init(padapter);
#endif /* RTW_BEAMFORMING_VERSION_2 */
#endif /* CONFIG_BEAMFORMING */

#ifdef CONFIG_RTW_REPEATER_SON
	init_rtw_rson_data(adapter_to_dvobj(padapter));
#endif

#ifdef CONFIG_RTW_80211K
	rtw_init_rm(padapter);
#endif

#ifdef CONFIG_RTW_CFGVENDOR_RANDOM_MAC_OUI
	memset(pwdev_priv->pno_mac_addr, 0xFF, ETH_ALEN);
#endif

exit:



	return ret8;

}

#ifdef CONFIG_WOWLAN
void rtw_cancel_dynamic_chk_timer(_adapter *padapter)
{
	_cancel_timer_ex(&adapter_to_dvobj(padapter)->dynamic_chk_timer);
}
#endif

void rtw_cancel_all_timer(_adapter *padapter)
{

	_cancel_timer_ex(&padapter->mlmepriv.assoc_timer);

	_cancel_timer_ex(&padapter->mlmepriv.scan_to_timer);

#ifdef CONFIG_DFS_MASTER
	_cancel_timer_ex(&adapter_to_rfctl(padapter)->radar_detect_timer);
#endif

	_cancel_timer_ex(&adapter_to_dvobj(padapter)->dynamic_chk_timer);
	_cancel_timer_ex(&adapter_to_dvobj(padapter)->periodic_tsf_update_end_timer);
#ifdef CONFIG_RTW_SW_LED
	/* cancel sw led timer */
	rtw_hal_sw_led_deinit(padapter);
#endif
	_cancel_timer_ex(&(adapter_to_pwrctl(padapter)->pwr_state_check_timer));

#ifdef CONFIG_TX_AMSDU
	_cancel_timer_ex(&padapter->xmitpriv.amsdu_bk_timer);
	_cancel_timer_ex(&padapter->xmitpriv.amsdu_be_timer);
	_cancel_timer_ex(&padapter->xmitpriv.amsdu_vo_timer);
	_cancel_timer_ex(&padapter->xmitpriv.amsdu_vi_timer);
#endif

#ifdef CONFIG_IOCTL_CFG80211
	_cancel_timer_ex(&padapter->rochinfo.remain_on_ch_timer);
#endif /* CONFIG_IOCTL_CFG80211 */

#ifdef CONFIG_SET_SCAN_DENY_TIMER
	_cancel_timer_ex(&padapter->mlmepriv.set_scan_deny_timer);
	rtw_clear_scan_deny(padapter);
#endif

#ifdef CONFIG_NEW_SIGNAL_STAT_PROCESS
	_cancel_timer_ex(&adapter_to_dvobj(padapter)->recvpriv.signal_stat_timer);
#endif

#ifdef CONFIG_LPS_RPWM_TIMER
	_cancel_timer_ex(&(adapter_to_pwrctl(padapter)->pwr_rpwm_timer));
#endif /* CONFIG_LPS_RPWM_TIMER */

#ifdef CONFIG_RTW_TOKEN_BASED_XMIT	
	_cancel_timer_ex(&padapter->mlmeextpriv.tbtx_xmit_timer);
	_cancel_timer_ex(&padapter->mlmeextpriv.tbtx_token_dispatch_timer);
#endif

	/* cancel dm timer */
	rtw_hal_dm_deinit(padapter);

#ifdef CONFIG_PLATFORM_FS_MX61
	msleep(50);
#endif
}

u8 rtw_free_drv_sw(_adapter *padapter)
{

#ifdef CONFIG_WAPI_SUPPORT
	rtw_wapi_free(padapter);
#endif

	/* we can call rtw_p2p_enable here, but: */
	/* 1. rtw_p2p_enable may have IO operation */
	/* 2. rtw_p2p_enable is bundled with wext interface */
	#ifdef CONFIG_P2P
	{
		struct wifidirect_info *pwdinfo = &padapter->wdinfo;
		#ifdef CONFIG_CONCURRENT_MODE
		struct roch_info *prochinfo = &padapter->rochinfo;
		#endif
		if (!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)) {
			_cancel_timer_ex(&pwdinfo->find_phase_timer);
			_cancel_timer_ex(&pwdinfo->restore_p2p_state_timer);
			_cancel_timer_ex(&pwdinfo->pre_tx_scan_timer);
			#ifdef CONFIG_CONCURRENT_MODE
			_cancel_timer_ex(&prochinfo->ap_roch_ch_switch_timer);
			#endif /* CONFIG_CONCURRENT_MODE */
			rtw_p2p_set_state(pwdinfo, P2P_STATE_NONE);
		}
	}
	#endif
	/* add for CONFIG_IEEE80211W, none 11w also can use */
	_rtw_spinlock_free(&padapter->security_key_mutex);

#ifdef CONFIG_BR_EXT
	_rtw_spinlock_free(&padapter->br_ext_lock);
#endif /* CONFIG_BR_EXT */

	free_mlme_ext_priv(&padapter->mlmeextpriv);

#ifdef CONFIG_TDLS
	/* rtw_free_tdls_info(&padapter->tdlsinfo); */
#endif /* CONFIG_TDLS */

#ifdef CONFIG_RTW_80211K
	rtw_free_rm_priv(padapter);
#endif

	// NEO : move to devobj_trx_resource_deinit()
	//rtw_free_cmd_priv(&padapter->cmdpriv);

	rtw_free_evt_priv(&padapter->evtpriv);

	rtw_free_mlme_priv(&padapter->mlmepriv);

	if (is_primary_adapter(padapter))
		rtw_rfctl_deinit(padapter);

	/* free_io_queue(padapter); */

	_rtw_free_xmit_priv(&padapter->xmitpriv);

	_rtw_free_sta_priv(&padapter->stapriv); /* will free bcmc_stainfo here */

	_rtw_free_recv_priv(&adapter_to_dvobj(padapter)->recvpriv);

	rtw_free_pwrctrl_priv(padapter);

#ifdef CONFIG_PLATFORM_CMAP_INTFS
	if (padapter->cmap_bss_status_evt) {
		cmap_intfs_mfree(padapter->cmap_bss_status_evt, padapter->cmap_bss_status_evt_len);
		padapter->cmap_bss_status_evt = NULL;
	}
#endif

	/* rtw_mfree((void *)padapter, sizeof (padapter)); */

	rtw_hal_free_data(padapter);

	return _SUCCESS;

}
void rtw_intf_start(_adapter *adapter)
{
	if (adapter->intf_start)
		adapter->intf_start(adapter);
	GET_HAL_DATA(adapter)->intf_start = 1;
}
void rtw_intf_stop(_adapter *adapter)
{
	if (adapter->intf_stop)
		adapter->intf_stop(adapter);
	GET_HAL_DATA(adapter)->intf_start = 0;
}

#ifdef CONFIG_CONCURRENT_MODE

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
static const struct net_device_ops rtw_netdev_vir_if_ops = {
	.ndo_init = rtw_ndev_init,
	.ndo_uninit = rtw_ndev_uninit,
	.ndo_open = netdev_open,
	.ndo_stop = netdev_close,
	.ndo_start_xmit = rtw_xmit_entry,
	.ndo_set_mac_address = rtw_net_set_mac_address,
	.ndo_get_stats = rtw_net_get_stats,
	.ndo_do_ioctl = rtw_ioctl,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	.ndo_select_queue	= rtw_select_queue,
#endif
};
#endif

static void rtw_hook_vir_if_ops(struct net_device *ndev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
	ndev->netdev_ops = &rtw_netdev_vir_if_ops;
#else
	ndev->init = rtw_ndev_init;
	ndev->uninit = rtw_ndev_uninit;
	ndev->open = netdev_open;
	ndev->stop = netdev_close;

	ndev->set_mac_address = rtw_net_set_mac_address;
#endif
}
_adapter *rtw_drv_add_vir_if(_adapter *primary_padapter,
	void (*set_intf_ops)(_adapter *primary_padapter, struct _io_ops *pops))
{
	int res = _FAIL;
	_adapter *padapter = NULL;
	struct dvobj_priv *pdvobjpriv;
	u8 mac[ETH_ALEN];
#ifdef CONFIG_MI_UNIQUE_MACADDR_BIT
	u32 mi_unique_macaddr_bit = 0;
	u8 i;
#endif

	/****** init adapter ******/
	padapter = (_adapter *)rtw_zvmalloc(sizeof(*padapter));
	if (padapter == NULL)
		goto exit;

	if (rtw_load_registry(padapter) != _SUCCESS)
		goto free_adapter;

	_rtw_memcpy(padapter, primary_padapter, sizeof(_adapter));

	/*  */
	padapter->bup = _FALSE;
	padapter->net_closed = _TRUE;
	padapter->dir_dev = NULL;
	padapter->dir_odm = NULL;

	/*set adapter_type/iface type*/
	padapter->isprimary = _FALSE;
	padapter->adapter_type = VIRTUAL_ADAPTER;

#ifdef CONFIG_MI_WITH_MBSSID_CAM
	padapter->hw_port = HW_PORT0;
#elif defined(CONFIG_PORT_BASED_TXBCN)
	padapter->hw_port = adapter_to_dvobj(padapter)->iface_nums;
#else
	padapter->hw_port = HW_PORT1;
#endif


	/****** hook vir if into dvobj ******/
	pdvobjpriv = adapter_to_dvobj(padapter);
	padapter->iface_id = pdvobjpriv->iface_nums;
	pdvobjpriv->padapters[pdvobjpriv->iface_nums++] = padapter;

	padapter->intf_start = primary_padapter->intf_start;
	padapter->intf_stop = primary_padapter->intf_stop;

	/* step init_io_priv */
	if ((rtw_init_io_priv(padapter, set_intf_ops)) == _FAIL) {
		goto free_adapter;
	}

	/*init drv data*/
	if (rtw_init_drv_sw(padapter) != _SUCCESS)
		goto free_drv_sw;


	/*get mac address from primary_padapter*/
	_rtw_memcpy(mac, adapter_mac_addr(primary_padapter), ETH_ALEN);

#ifdef CONFIG_MI_UNIQUE_MACADDR_BIT
	mi_unique_macaddr_bit = BIT(CONFIG_MI_UNIQUE_MACADDR_BIT) >> 24;
	/* Find out CONFIG_MI_UNIQUE_MACADDR_BIT in which nic specific byte */
	for(i=3;i<6;i++) {
		if((mi_unique_macaddr_bit >> 8) == 0)
			break;

		mi_unique_macaddr_bit >>= 8;
	}

	if((mac[i] & (u8)mi_unique_macaddr_bit)== 0) {
		RTW_INFO("%s() "MAC_FMT" : BIT%u is zero\n", __func__, MAC_ARG(mac), CONFIG_MI_UNIQUE_MACADDR_BIT);
		/* IFACE_ID1/IFACE_ID3 : set locally administered bit */
		if(padapter->iface_id & BIT(0))
			mac[0] |= BIT(1);
		/* IFACE_ID2/IFACE_ID3 : set bit(CONFIG_MI_UNIQUE_MACADDR_BIT) */
		if(padapter->iface_id >> 1)
			mac[i] |= (u8)mi_unique_macaddr_bit;
	} else
#endif
	{
	/*
	* If the BIT1 is 0, the address is universally administered.
	* If it is 1, the address is locally administered
	*/
	mac[0] |= BIT(1);
	if (padapter->iface_id > IFACE_ID1)
		mac[0] ^= ((padapter->iface_id)<<2);
	}

	_rtw_memcpy(adapter_mac_addr(padapter), mac, ETH_ALEN);
	/* update mac-address to mbsid-cam cache*/
#ifdef CONFIG_MI_WITH_MBSSID_CAM
	rtw_mbid_camid_alloc(padapter, adapter_mac_addr(padapter));
#endif
	RTW_INFO("%s if%d mac_addr : "MAC_FMT"\n", __func__, padapter->iface_id + 1, MAC_ARG(adapter_mac_addr(padapter)));
#ifdef CONFIG_P2P
	rtw_init_wifidirect_addrs(padapter, adapter_mac_addr(padapter), adapter_mac_addr(padapter));
#endif

	rtw_led_set_ctl_en_mask_virtual(padapter);
	rtw_led_set_iface_en(padapter, 1);

	res = _SUCCESS;

free_drv_sw:
	if (res != _SUCCESS && padapter)
		rtw_free_drv_sw(padapter);
free_adapter:
	if (res != _SUCCESS && padapter) {
		rtw_vmfree((u8 *)padapter, sizeof(*padapter));
		padapter = NULL;
	}
exit:
	return padapter;
}

void rtw_drv_stop_vir_if(_adapter *padapter)
{
	struct net_device *pnetdev = NULL;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	if (padapter == NULL)
		return;
	RTW_INFO(FUNC_ADPT_FMT" enter\n", FUNC_ADPT_ARG(padapter));

	pnetdev = padapter->pnetdev;

	if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE))
		rtw_disassoc_cmd(padapter, 0, RTW_CMDF_DIRECTLY);

#ifdef CONFIG_AP_MODE
	if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter)) {
		free_mlme_ap_info(padapter);
		#ifdef CONFIG_HOSTAPD_MLME
		hostapd_mode_unload(padapter);
		#endif
	}
#endif

	if (padapter->bup == _TRUE) {
		#ifdef CONFIG_XMIT_ACK
		if (padapter->xmitpriv.ack_tx)
			rtw_ack_tx_done(&padapter->xmitpriv, RTW_SCTX_DONE_DRV_STOP);
		#endif

		rtw_intf_stop(padapter);
		padapter->bup = _FALSE;
	}
	rtw_stop_drv_threads(padapter);
	/* cancel timer after thread stop */
	rtw_cancel_all_timer(padapter);
}

void rtw_drv_free_vir_if(_adapter *padapter)
{
	if (padapter == NULL)
		return;

	RTW_INFO(FUNC_ADPT_FMT"\n", FUNC_ADPT_ARG(padapter));
	rtw_free_drv_sw(padapter);

	/* TODO: use rtw_os_ndevs_deinit instead at the first stage of driver's dev deinit function */
	rtw_os_ndev_free(padapter);

	rtw_vmfree((u8 *)padapter, sizeof(_adapter));
}


void rtw_drv_stop_vir_ifaces(struct dvobj_priv *dvobj)
{
	int i;

	for (i = VIF_START_ID; i < dvobj->iface_nums; i++)
		rtw_drv_stop_vir_if(dvobj->padapters[i]);
}

void rtw_drv_free_vir_ifaces(struct dvobj_priv *dvobj)
{
	int i;

	for (i = VIF_START_ID; i < dvobj->iface_nums; i++)
		rtw_drv_free_vir_if(dvobj->padapters[i]);
}


#endif /*end of CONFIG_CONCURRENT_MODE*/

/* IPv4, IPv6 IP addr notifier */
static int rtw_inetaddr_notifier_call(struct notifier_block *nb,
				      unsigned long action, void *data)
{
	struct in_ifaddr *ifa = (struct in_ifaddr *)data;
	struct net_device *ndev;
	struct mlme_ext_priv *pmlmeext = NULL;
	struct mlme_ext_info *pmlmeinfo = NULL;
	_adapter *adapter = NULL;

	if (!ifa || !ifa->ifa_dev || !ifa->ifa_dev->dev)
		return NOTIFY_DONE;

	ndev = ifa->ifa_dev->dev;

	if (!is_rtw_ndev(ndev))
		return NOTIFY_DONE;

	adapter = (_adapter *)rtw_netdev_priv(ifa->ifa_dev->dev);

	if (adapter == NULL)
		return NOTIFY_DONE;

	pmlmeext = &adapter->mlmeextpriv;
	pmlmeinfo = &pmlmeext->mlmext_info;

	switch (action) {
	case NETDEV_UP:
		_rtw_memcpy(pmlmeinfo->ip_addr, &ifa->ifa_address,
					RTW_IP_ADDR_LEN);
		RTW_DBG("%s[%s]: up IP: %pI4\n", __func__,
					ifa->ifa_label, pmlmeinfo->ip_addr);
	break;
	case NETDEV_DOWN:
		_rtw_memset(pmlmeinfo->ip_addr, 0, RTW_IP_ADDR_LEN);
		RTW_DBG("%s[%s]: down IP: %pI4\n", __func__,
					ifa->ifa_label, pmlmeinfo->ip_addr);
	break;
	default:
		RTW_DBG("%s: default action\n", __func__);
	break;
	}
	return NOTIFY_DONE;
}

#ifdef CONFIG_IPV6
static int rtw_inet6addr_notifier_call(struct notifier_block *nb,
				       unsigned long action, void *data)
{
	struct inet6_ifaddr *inet6_ifa = data;
	struct net_device *ndev;
	struct pwrctrl_priv *pwrctl = NULL;
	struct mlme_ext_priv *pmlmeext = NULL;
	struct mlme_ext_info *pmlmeinfo = NULL;
	_adapter *adapter = NULL;

	if (!inet6_ifa || !inet6_ifa->idev || !inet6_ifa->idev->dev)
		return NOTIFY_DONE;

	ndev = inet6_ifa->idev->dev;

	if (!is_rtw_ndev(ndev))
		return NOTIFY_DONE;

	adapter = (_adapter *)rtw_netdev_priv(inet6_ifa->idev->dev);

	if (adapter == NULL)
		return NOTIFY_DONE;

	pmlmeext =  &adapter->mlmeextpriv;
	pmlmeinfo = &pmlmeext->mlmext_info;
	pwrctl = adapter_to_pwrctl(adapter);

	pmlmeext = &adapter->mlmeextpriv;
	pmlmeinfo = &pmlmeext->mlmext_info;

	switch (action) {
	case NETDEV_UP:
#ifdef CONFIG_WOWLAN
		pwrctl->wowlan_ns_offload_en = _TRUE;
#endif
		_rtw_memcpy(pmlmeinfo->ip6_addr, &inet6_ifa->addr,
					RTW_IPv6_ADDR_LEN);
		RTW_DBG("%s: up IPv6 addrs: %pI6\n", __func__,
					pmlmeinfo->ip6_addr);
			break;
	case NETDEV_DOWN:
#ifdef CONFIG_WOWLAN
		pwrctl->wowlan_ns_offload_en = _FALSE;
#endif
		_rtw_memset(pmlmeinfo->ip6_addr, 0, RTW_IPv6_ADDR_LEN);
		RTW_DBG("%s: down IPv6 addrs: %pI6\n", __func__,
					pmlmeinfo->ip6_addr);
		break;
	default:
		RTW_DBG("%s: default action\n", __func__);
		break;
	}
	return NOTIFY_DONE;
}
#endif

static struct notifier_block rtw_inetaddr_notifier = {
	.notifier_call = rtw_inetaddr_notifier_call
};

#ifdef CONFIG_IPV6
static struct notifier_block rtw_inet6addr_notifier = {
	.notifier_call = rtw_inet6addr_notifier_call
};
#endif

void rtw_inetaddr_notifier_register(void)
{
	RTW_INFO("%s\n", __func__);
	register_inetaddr_notifier(&rtw_inetaddr_notifier);
#ifdef CONFIG_IPV6
	register_inet6addr_notifier(&rtw_inet6addr_notifier);
#endif
}

void rtw_inetaddr_notifier_unregister(void)
{
	RTW_INFO("%s\n", __func__);
	unregister_inetaddr_notifier(&rtw_inetaddr_notifier);
#ifdef CONFIG_IPV6
	unregister_inet6addr_notifier(&rtw_inet6addr_notifier);
#endif
}

int rtw_os_ndevs_register(struct dvobj_priv *dvobj)
{
	int i, status = _SUCCESS;
	struct registry_priv *regsty = dvobj_to_regsty(dvobj);
	_adapter *adapter;

#if defined(CONFIG_IOCTL_CFG80211)
	if (rtw_cfg80211_dev_res_register(dvobj) != _SUCCESS) {
		rtw_warn_on(1);
		return _FAIL;
	}
#endif

	for (i = 0; i < dvobj->iface_nums; i++) {

		if (i >= CONFIG_IFACE_NUMBER) {
			RTW_ERR("%s %d >= CONFIG_IFACE_NUMBER(%d)\n", __func__, i, CONFIG_IFACE_NUMBER);
			rtw_warn_on(1);
			continue;
		}

		adapter = dvobj->padapters[i];
		if (adapter) {
			char *name;

			#ifdef CONFIG_RTW_DYNAMIC_NDEV
			if (!is_primary_adapter(adapter))
				continue;
			#endif

			if (adapter->iface_id == IFACE_ID0)
				name = regsty->ifname;
			else if (adapter->iface_id == IFACE_ID1)
				name = regsty->if2name;
			else
				name = "wlan%d";

			status = rtw_os_ndev_register(adapter, name);

			if (status != _SUCCESS) {
				rtw_warn_on(1);
				break;
			}
		}
	}

	if (status != _SUCCESS) {
		for (; i >= 0; i--) {
			adapter = dvobj->padapters[i];
			if (adapter)
				rtw_os_ndev_unregister(adapter);
		}
	}

#if defined(CONFIG_IOCTL_CFG80211)
	if (status != _SUCCESS)
		rtw_cfg80211_dev_res_unregister(dvobj);
#endif
	return status;
}

void rtw_os_ndevs_unregister(struct dvobj_priv *dvobj)
{
	int i;
	_adapter *adapter = NULL;

	for (i = 0; i < dvobj->iface_nums; i++) {
		adapter = dvobj->padapters[i];

		if (adapter == NULL)
			continue;

		rtw_os_ndev_unregister(adapter);
	}

#if defined(CONFIG_IOCTL_CFG80211)
	rtw_cfg80211_dev_res_unregister(dvobj);
#endif
}

/**
 * rtw_os_ndevs_init - Allocate and register OS layer net devices and relating structures for @dvobj
 * @dvobj: the dvobj on which this function applies
 *
 * Returns:
 * _SUCCESS or _FAIL
 */
int rtw_os_ndevs_init(struct dvobj_priv *dvobj)
{
	int ret = _FAIL;

	if (rtw_os_ndevs_alloc(dvobj) != _SUCCESS)
		goto exit;

	if (rtw_os_ndevs_register(dvobj) != _SUCCESS)
		goto os_ndevs_free;

	ret = _SUCCESS;

os_ndevs_free:
	if (ret != _SUCCESS)
		rtw_os_ndevs_free(dvobj);
exit:
	return ret;
}

/**
 * rtw_os_ndevs_deinit - Unregister and free OS layer net devices and relating structures for @dvobj
 * @dvobj: the dvobj on which this function applies
 */
void rtw_os_ndevs_deinit(struct dvobj_priv *dvobj)
{
	rtw_os_ndevs_unregister(dvobj);
	rtw_os_ndevs_free(dvobj);
}

#ifdef CONFIG_BR_EXT
void netdev_br_init(struct net_device *netdev)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(netdev);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	rcu_read_lock();
#endif

	/* if(check_fwstate(pmlmepriv, WIFI_STATION_STATE|WIFI_ADHOC_STATE) == _TRUE) */
	{
		/* struct net_bridge	*br = netdev->br_port->br; */ /* ->dev->dev_addr; */
		#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 35))
		if (netdev->br_port)
		#else
		if (rcu_dereference(adapter->pnetdev->rx_handler_data))
		#endif
		{
			struct net_device *br_netdev;

			br_netdev = rtw_get_bridge_ndev_by_name(CONFIG_BR_EXT_BRNAME);
			if (br_netdev) {
				memcpy(adapter->br_mac, br_netdev->dev_addr, ETH_ALEN);
				dev_put(br_netdev);
				RTW_INFO(FUNC_NDEV_FMT" bind bridge dev "NDEV_FMT"("MAC_FMT")\n"
					, FUNC_NDEV_ARG(netdev), NDEV_ARG(br_netdev), MAC_ARG(br_netdev->dev_addr));
			} else {
				RTW_INFO(FUNC_NDEV_FMT" can't get bridge dev by name \"%s\"\n"
					, FUNC_NDEV_ARG(netdev), CONFIG_BR_EXT_BRNAME);
			}
		}

		adapter->ethBrExtInfo.addPPPoETag = 1;
	}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	rcu_read_unlock();
#endif
}
#endif /* CONFIG_BR_EXT */

static int _netdev_open(struct net_device *pnetdev)
{
	uint status;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);

	RTW_INFO(FUNC_NDEV_FMT" start\n", FUNC_NDEV_ARG(pnetdev));

	if (!rtw_is_hw_init_completed(padapter)) { // ips 
		dev_clr_surprise_removed(dvobj);
		dev_clr_drv_stopped(dvobj);
		RTW_ENABLE_FUNC(dvobj, DF_RX_BIT);
		RTW_ENABLE_FUNC(dvobj, DF_TX_BIT);
		status = rtk_hal_init(padapter);
		if (status == _FAIL)
			goto netdev_open_error;
		rtw_led_control(padapter, LED_CTL_NO_LINK);
		#ifndef RTW_HALMAC
		status = rtw_mi_start_drv_threads(padapter);
		if (status == _FAIL) {
			RTW_ERR(FUNC_NDEV_FMT "Initialize driver thread failed!\n", FUNC_NDEV_ARG(pnetdev));
			goto netdev_open_error;
		}

		rtw_intf_start(GET_PRIMARY_ADAPTER(padapter));
		#endif /* !RTW_HALMAC */

		{
	#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
			_adapter *prim_adpt = GET_PRIMARY_ADAPTER(padapter);
		
			if (prim_adpt && (_TRUE == prim_adpt->EEPROMBluetoothCoexist)) {
				rtw_btcoex_init_socket(prim_adpt);
				prim_adpt->coex_info.BtMgnt.ExtConfig.HCIExtensionVer = 0x04;
				rtw_btcoex_SetHciVersion(prim_adpt, 0x04);
			}
	#endif /* CONFIG_BT_COEXIST_SOCKET_TRX */

			_set_timer(&adapter_to_dvobj(padapter)->dynamic_chk_timer, 2000);

	#ifndef CONFIG_IPS_CHECK_IN_WD
			rtw_set_pwr_state_check_timer(pwrctrlpriv);
	#endif /*CONFIG_IPS_CHECK_IN_WD*/
		}

	}

	/*if (padapter->bup == _FALSE) */
	{
		rtw_hal_iface_init(padapter);

		#ifdef CONFIG_RTW_NAPI
		if(padapter->napi_state == NAPI_DISABLE) {
			napi_enable(&padapter->napi);
			padapter->napi_state = NAPI_ENABLE;
		}
		#endif

		#ifdef CONFIG_IOCTL_CFG80211
		rtw_cfg80211_init_wdev_data(padapter);
		#endif
		/* rtw_netif_carrier_on(pnetdev); */ /* call this func when rtw_joinbss_event_callback return success */
		rtw_netif_wake_queue(pnetdev);

		#ifdef CONFIG_BR_EXT
		if (is_primary_adapter(padapter))
			netdev_br_init(pnetdev);
		#endif /* CONFIG_BR_EXT */


		padapter->bup = _TRUE;
		padapter->net_closed = _FALSE;
		padapter->netif_up = _TRUE;
		pwrctrlpriv->bips_processing = _FALSE;
	}

	RTW_INFO(FUNC_NDEV_FMT" Success (bup=%d)\n", FUNC_NDEV_ARG(pnetdev), padapter->bup);
	return 0;

netdev_open_error:
	padapter->bup = _FALSE;

	#ifdef CONFIG_RTW_NAPI
	if(padapter->napi_state == NAPI_ENABLE) {
		napi_disable(&padapter->napi);
		padapter->napi_state = NAPI_DISABLE;
	}
	#endif

	rtw_netif_carrier_off(pnetdev);
	rtw_netif_stop_queue(pnetdev);

	RTW_ERR(FUNC_NDEV_FMT" Failed!! (bup=%d)\n", FUNC_NDEV_ARG(pnetdev), padapter->bup);

	return -1;

}

int netdev_open(struct net_device *pnetdev)
{
	int ret = _FALSE;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);

	if (pwrctrlpriv->bInSuspend == _TRUE) {
		RTW_INFO(" [WARN] "ADPT_FMT" %s  failed, bInSuspend=%d\n", ADPT_ARG(padapter), __func__, pwrctrlpriv->bInSuspend);
		return 0;
	}


	RTW_INFO(FUNC_NDEV_FMT" , netif_up=%d\n", FUNC_NDEV_ARG(pnetdev), padapter->netif_up);
	/*rtw_dump_stack();*/
	_rtw_mutex_lock_interruptible(&(adapter_to_dvobj(padapter)->hw_init_mutex));
	ret = _netdev_open(pnetdev);
	_rtw_mutex_unlock(&(adapter_to_dvobj(padapter)->hw_init_mutex));


#ifdef CONFIG_AUTO_AP_MODE
	if (padapter->iface_id == IFACE_ID2)
		rtw_start_auto_ap(padapter);
#endif

	return ret;
}

#ifdef CONFIG_IPS
int  ips_netdrv_open(_adapter *padapter)
{
	int status = _SUCCESS;
	/* struct pwrctrl_priv	*pwrpriv = adapter_to_pwrctl(padapter); */

	padapter->net_closed = _FALSE;

	RTW_INFO("===> %s.........\n", __FUNCTION__);


	rtw_clr_drv_stopped(padapter);
	/* padapter->bup = _TRUE; */
	if (!rtw_is_hw_init_completed(padapter)) {
		status = rtk_hal_init(padapter);
		if (status == _FAIL) {
			goto netdev_open_error;
		}
		rtw_mi_hal_iface_init(padapter);
	}
#if 0
	rtw_mi_set_mac_addr(padapter);
#endif
#ifndef RTW_HALMAC
	rtw_intf_start(padapter);
#endif /* !RTW_HALMAC */

#ifndef CONFIG_IPS_CHECK_IN_WD
	rtw_set_pwr_state_check_timer(adapter_to_pwrctl(padapter));
#endif
	_set_timer(&adapter_to_dvobj(padapter)->dynamic_chk_timer, 2000);

	return _SUCCESS;

netdev_open_error:
	/* padapter->bup = _FALSE; */
	RTW_INFO("-ips_netdrv_open - drv_open failure, bup=%d\n", padapter->bup);

	return _FAIL;
}

int rtw_ips_pwr_up(_adapter *padapter)
{
	int result;
#if defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS)
#ifdef DBG_CONFIG_ERROR_DETECT
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;
#endif/* #ifdef DBG_CONFIG_ERROR_DETECT */
#endif /* defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS) */
	systime start_time = rtw_get_current_time();
	RTW_INFO("===>  rtw_ips_pwr_up..............\n");

#if defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS)
#ifdef DBG_CONFIG_ERROR_DETECT
	if (psrtpriv->silent_reset_inprogress == _TRUE)
#endif/* #ifdef DBG_CONFIG_ERROR_DETECT */
#endif /* defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS) */
		rtw_reset_drv_sw(padapter);

	result = ips_netdrv_open(padapter);

	rtw_led_control(padapter, LED_CTL_NO_LINK);

	RTW_INFO("<===  rtw_ips_pwr_up.............. in %dms\n", rtw_get_passing_time_ms(start_time));
	return result;

}

void rtw_ips_pwr_down(_adapter *padapter)
{
	systime start_time = rtw_get_current_time();
	RTW_INFO("===> rtw_ips_pwr_down...................\n");

	padapter->net_closed = _TRUE;

	rtw_ips_dev_unload(padapter);
	RTW_INFO("<=== rtw_ips_pwr_down..................... in %dms\n", rtw_get_passing_time_ms(start_time));
}
#endif
void rtw_ips_dev_unload(_adapter *padapter)
{
#if defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS)
#ifdef DBG_CONFIG_ERROR_DETECT
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;
#endif/* #ifdef DBG_CONFIG_ERROR_DETECT */
#endif /* defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS) */
	RTW_INFO("====> %s...\n", __FUNCTION__);


#if defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS)
#ifdef DBG_CONFIG_ERROR_DETECT
	if (psrtpriv->silent_reset_inprogress == _TRUE)
#endif /* #ifdef DBG_CONFIG_ERROR_DETECT */
#endif /* defined(CONFIG_SWLPS_IN_IPS) || defined(CONFIG_FWLPS_IN_IPS) */
	{
		rtw_hal_set_hwreg(padapter, HW_VAR_FIFO_CLEARN_UP, 0);
		rtw_intf_stop(padapter);
	}

	if (!rtw_is_surprise_removed(padapter))
		rtk_hal_deinit(padapter);

}
int _pm_netdev_open(_adapter *padapter)
{
	uint status;
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);
	struct net_device *pnetdev = padapter->pnetdev;

	RTW_INFO(FUNC_NDEV_FMT" start\n", FUNC_NDEV_ARG(pnetdev));

	if (!rtw_is_hw_init_completed(padapter)) { // ips 
		rtw_clr_surprise_removed(padapter);
		rtw_clr_drv_stopped(padapter);
		status = rtk_hal_init(padapter);
		if (status == _FAIL)
			goto netdev_open_error;
		rtw_led_control(padapter, LED_CTL_NO_LINK);
		#ifndef RTW_HALMAC
		status = rtw_mi_start_drv_threads(padapter);
		if (status == _FAIL) {
			RTW_ERR(FUNC_NDEV_FMT "Initialize driver thread failed!\n", FUNC_NDEV_ARG(pnetdev));
			goto netdev_open_error;
		}

		rtw_intf_start(GET_PRIMARY_ADAPTER(padapter));
		#endif /* !RTW_HALMAC */

		{
			_set_timer(&adapter_to_dvobj(padapter)->dynamic_chk_timer, 2000);

	#ifndef CONFIG_IPS_CHECK_IN_WD
			rtw_set_pwr_state_check_timer(pwrctrlpriv);
	#endif /*CONFIG_IPS_CHECK_IN_WD*/
		}

	}

	/*if (padapter->bup == _FALSE) */
	{
		rtw_hal_iface_init(padapter);

		padapter->bup = _TRUE;
		padapter->net_closed = _FALSE;
		padapter->netif_up = _TRUE;
		pwrctrlpriv->bips_processing = _FALSE;
	}

	RTW_INFO(FUNC_NDEV_FMT" Success (bup=%d)\n", FUNC_NDEV_ARG(pnetdev), padapter->bup);
	return 0;

netdev_open_error:
	padapter->bup = _FALSE;

	rtw_netif_carrier_off(pnetdev);
	rtw_netif_stop_queue(pnetdev);

	RTW_ERR(FUNC_NDEV_FMT" Failed!! (bup=%d)\n", FUNC_NDEV_ARG(pnetdev), padapter->bup);

	return -1;

}
int _mi_pm_netdev_open(struct net_device *pnetdev)
{
	int i;
	int status = 0;
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	_adapter *iface;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if (iface->netif_up) {
			status = _pm_netdev_open(iface);
			if (status == -1) {
				RTW_ERR("%s failled\n", __func__);
				break;
			}
		}
	}

	return status;
}
int pm_netdev_open(struct net_device *pnetdev, u8 bnormal)
{
	int status = 0;

	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);

	if (_TRUE == bnormal) {
		_enter_critical_mutex(&(adapter_to_dvobj(padapter)->hw_init_mutex), NULL);
		status = _mi_pm_netdev_open(pnetdev);
		_exit_critical_mutex(&(adapter_to_dvobj(padapter)->hw_init_mutex), NULL);
	}
#ifdef CONFIG_IPS
	else
		status = (_SUCCESS == ips_netdrv_open(padapter)) ? (0) : (-1);
#endif

	return status;
}
#ifdef CONFIG_CLIENT_PORT_CFG
extern void rtw_hw_client_port_release(_adapter *adapter);
#endif
static int netdev_close(struct net_device *pnetdev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct pwrctrl_priv *pwrctl = adapter_to_pwrctl(padapter);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(padapter);
#endif /* CONFIG_BT_COEXIST_SOCKET_TRX */

	RTW_INFO(FUNC_NDEV_FMT" , bup=%d\n", FUNC_NDEV_ARG(pnetdev), padapter->bup);
#ifndef CONFIG_PLATFORM_INTEL_BYT
	padapter->net_closed = _TRUE;
	padapter->netif_up = _FALSE;
	pmlmepriv->LinkDetectInfo.bBusyTraffic = _FALSE;

#ifdef CONFIG_CLIENT_PORT_CFG
	if (MLME_IS_STA(padapter))
		rtw_hw_client_port_release(padapter);
#endif
	/*	if (!rtw_is_hw_init_completed(padapter)) {
			RTW_INFO("(1)871x_drv - drv_close, bup=%d, hw_init_completed=%s\n", padapter->bup, rtw_is_hw_init_completed(padapter)?"_TRUE":"_FALSE");

			rtw_set_drv_stopped(padapter);

			rtw_dev_unload(padapter);
		}
		else*/
	if (pwrctl->rf_pwrstate == rf_on) {
		RTW_INFO("(2)871x_drv - drv_close, bup=%d, hw_init_completed=%s\n", padapter->bup, rtw_is_hw_init_completed(padapter) ? "_TRUE" : "_FALSE");

		/* s1. */
		if (pnetdev)
			rtw_netif_stop_queue(pnetdev);

#ifndef CONFIG_RTW_ANDROID
		/* s2. */
		LeaveAllPowerSaveMode(padapter);
		rtw_disassoc_cmd(padapter, 500, RTW_CMDF_WAIT_ACK);
		/* s2-2.  indicate disconnect to os */
		rtw_indicate_disconnect(padapter, 0, _FALSE);
		/* s2-3. */
		rtw_free_assoc_resources_cmd(padapter, _TRUE, RTW_CMDF_WAIT_ACK);
		/* s2-4. */
		rtw_free_network_queue(padapter, _TRUE);
#endif
	}

#ifdef CONFIG_BR_EXT
	/* if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE)) */
	{
		/* void nat25_db_cleanup(_adapter *priv); */
		nat25_db_cleanup(padapter);
	}
#endif /* CONFIG_BR_EXT */

#ifdef CONFIG_P2P
	if (!rtw_p2p_chk_role(&padapter->wdinfo, P2P_ROLE_DISABLE))
		rtw_p2p_enable(padapter, P2P_ROLE_DISABLE);
#endif /* CONFIG_P2P */

	rtw_scan_abort(padapter); /* stop scanning process before wifi is going to down */
#ifdef CONFIG_IOCTL_CFG80211
	rtw_cfg80211_wait_scan_req_empty(padapter, 200);
	adapter_wdev_data(padapter)->bandroid_scan = _FALSE;
	/* padapter->rtw_wdev->iftype = NL80211_IFTYPE_MONITOR; */ /* set this at the end */
#endif /* CONFIG_IOCTL_CFG80211 */

#ifdef CONFIG_WAPI_SUPPORT
	rtw_wapi_disable_tx(padapter);
#endif
#ifdef CONFIG_BT_COEXIST_SOCKET_TRX
	if (is_primary_adapter(padapter) && (_TRUE == pHalData->EEPROMBluetoothCoexist))
		rtw_btcoex_close_socket(padapter);
	else
		RTW_INFO("CONFIG_BT_COEXIST: VIRTUAL_ADAPTER\n");
#endif /* CONFIG_BT_COEXIST_SOCKET_TRX */
#else /* !CONFIG_PLATFORM_INTEL_BYT */

	if (pwrctl->bInSuspend == _TRUE) {
		RTW_INFO("+871x_drv - drv_close, bInSuspend=%d\n", pwrctl->bInSuspend);
		return 0;
	}

	rtw_scan_abort(padapter); /* stop scanning process before wifi is going to down */
#ifdef CONFIG_IOCTL_CFG80211
	rtw_cfg80211_wait_scan_req_empty(padapter, 200);
#endif

	RTW_INFO("netdev_close, bips_processing=%d\n", pwrctl->bips_processing);
	while (pwrctl->bips_processing == _TRUE) /* waiting for ips_processing done before call rtw_dev_unload() */
		rtw_msleep_os(1);

	rtw_dev_unload(padapter);
	rtw_sdio_set_power(0);

#endif /* !CONFIG_PLATFORM_INTEL_BYT */

	RTW_INFO("-871x_drv - drv_close, bup=%d\n", padapter->bup);

	return 0;

}

int pm_netdev_close(struct net_device *pnetdev, u8 bnormal)
{
	int status = 0;

	status = netdev_close(pnetdev);

	return status;
}

void rtw_ndev_destructor(struct net_device *ndev)
{
	RTW_INFO(FUNC_NDEV_FMT"\n", FUNC_NDEV_ARG(ndev));

#ifdef CONFIG_IOCTL_CFG80211
	if (ndev->ieee80211_ptr)
		rtw_mfree((u8 *)ndev->ieee80211_ptr, sizeof(struct wireless_dev));
#endif
	free_netdev(ndev);
}

#ifdef CONFIG_ARP_KEEP_ALIVE
struct route_info {
	struct in_addr dst_addr;
	struct in_addr src_addr;
	struct in_addr gateway;
	unsigned int dev_index;
};

static void parse_routes(struct nlmsghdr *nl_hdr, struct route_info *rt_info)
{
	struct rtmsg *rt_msg;
	struct rtattr *rt_attr;
	int rt_len;

	rt_msg = (struct rtmsg *) NLMSG_DATA(nl_hdr);
	if ((rt_msg->rtm_family != AF_INET) || (rt_msg->rtm_table != RT_TABLE_MAIN))
		return;

	rt_attr = (struct rtattr *) RTM_RTA(rt_msg);
	rt_len = RTM_PAYLOAD(nl_hdr);

	for (; RTA_OK(rt_attr, rt_len); rt_attr = RTA_NEXT(rt_attr, rt_len)) {
		switch (rt_attr->rta_type) {
		case RTA_OIF:
			rt_info->dev_index = *(int *) RTA_DATA(rt_attr);
			break;
		case RTA_GATEWAY:
			rt_info->gateway.s_addr = *(u_int *) RTA_DATA(rt_attr);
			break;
		case RTA_PREFSRC:
			rt_info->src_addr.s_addr = *(u_int *) RTA_DATA(rt_attr);
			break;
		case RTA_DST:
			rt_info->dst_addr.s_addr = *(u_int *) RTA_DATA(rt_attr);
			break;
		}
	}
}

static int route_dump(u32 *gw_addr , int *gw_index)
{
	int err = 0;
	struct socket *sock;
	struct {
		struct nlmsghdr nlh;
		struct rtgenmsg g;
	} req;
	struct msghdr msg;
	struct iovec iov;
	struct sockaddr_nl nladdr;
	mm_segment_t oldfs;
	char *pg;
	int size = 0;

	err = sock_create(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE, &sock);
	if (err) {
		printk(": Could not create a datagram socket, error = %d\n", -ENXIO);
		return err;
	}

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = RTM_GETROUTE;
	req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
	req.nlh.nlmsg_pid = 0;
	req.g.rtgen_family = AF_INET;

	iov.iov_base = &req;
	iov.iov_len = sizeof(req);

	msg.msg_name = &nladdr;
	msg.msg_namelen = sizeof(nladdr);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
	/* referece:sock_xmit in kernel code
	 * WRITE for sock_sendmsg, READ for sock_recvmsg
	 * third parameter for msg_iovlen
	 * last parameter for iov_len
	 */
	iov_iter_init(&msg.msg_iter, WRITE, &iov, 1, sizeof(req));
#else
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
#endif
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = MSG_DONTWAIT;

	oldfs = get_fs();
	set_fs(KERNEL_DS);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
	err = sock_sendmsg(sock, &msg);
#else
	err = sock_sendmsg(sock, &msg, sizeof(req));
#endif
	set_fs(oldfs);

	if (err < 0)
		goto out_sock;

	pg = (char *) __get_free_page(GFP_KERNEL);
	if (pg == NULL) {
		err = -ENOMEM;
		goto out_sock;
	}

#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
restart:
#endif

	for (;;) {
		struct nlmsghdr *h;

		iov.iov_base = pg;
		iov.iov_len = PAGE_SIZE;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
		iov_iter_init(&msg.msg_iter, READ, &iov, 1, PAGE_SIZE);
#endif

		oldfs = get_fs();
		set_fs(KERNEL_DS);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 7, 0))
		err = sock_recvmsg(sock, &msg, MSG_DONTWAIT);
#else
		err = sock_recvmsg(sock, &msg, PAGE_SIZE, MSG_DONTWAIT);
#endif
		set_fs(oldfs);

		if (err < 0)
			goto out_sock_pg;

		if (msg.msg_flags & MSG_TRUNC) {
			err = -ENOBUFS;
			goto out_sock_pg;
		}

		h = (struct nlmsghdr *) pg;

		while (NLMSG_OK(h, err)) {
			struct route_info rt_info;
			if (h->nlmsg_type == NLMSG_DONE) {
				err = 0;
				goto done;
			}

			if (h->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *errm = (struct nlmsgerr *) NLMSG_DATA(h);
				err = errm->error;
				printk("NLMSG error: %d\n", errm->error);
				goto done;
			}

			if (h->nlmsg_type == RTM_GETROUTE)
				printk("RTM_GETROUTE: NLMSG: %d\n", h->nlmsg_type);
			if (h->nlmsg_type != RTM_NEWROUTE) {
				printk("NLMSG: %d\n", h->nlmsg_type);
				err = -EINVAL;
				goto done;
			}

			memset(&rt_info, 0, sizeof(struct route_info));
			parse_routes(h, &rt_info);
			if (!rt_info.dst_addr.s_addr && rt_info.gateway.s_addr && rt_info.dev_index) {
				*gw_addr = rt_info.gateway.s_addr;
				*gw_index = rt_info.dev_index;

			}
			h = NLMSG_NEXT(h, err);
		}

		if (err) {
			printk("!!!Remnant of size %d %d %d\n", err, h->nlmsg_len, h->nlmsg_type);
			err = -EINVAL;
			break;
		}
	}

done:
#if defined(CONFIG_IPV6) || defined(CONFIG_IPV6_MODULE)
	if (!err && req.g.rtgen_family == AF_INET) {
		req.g.rtgen_family = AF_INET6;

		iov.iov_base = &req;
		iov.iov_len = sizeof(req);

		msg.msg_name = &nladdr;
		msg.msg_namelen = sizeof(nladdr);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 19, 0))
		iov_iter_init(&msg.msg_iter, WRITE, &iov, 1, sizeof(req));
#else
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
#endif
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		msg.msg_flags = MSG_DONTWAIT;

		oldfs = get_fs();
		set_fs(KERNEL_DS);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
		err = sock_sendmsg(sock, &msg);
#else
		err = sock_sendmsg(sock, &msg, sizeof(req));
#endif
		set_fs(oldfs);

		if (err > 0)
			goto restart;
	}
#endif

out_sock_pg:
	free_page((unsigned long) pg);

out_sock:
	sock_release(sock);
	return err;
}

static int arp_query(unsigned char *haddr, u32 paddr,
		     struct net_device *dev)
{
	struct neighbour *neighbor_entry;
	int	ret = 0;

	neighbor_entry = neigh_lookup(&arp_tbl, &paddr, dev);

	if (neighbor_entry != NULL) {
		neighbor_entry->used = jiffies;
		if (neighbor_entry->nud_state & NUD_VALID) {
			_rtw_memcpy(haddr, neighbor_entry->ha, dev->addr_len);
			ret = 1;
		}
		neigh_release(neighbor_entry);
	}
	return ret;
}

static int get_defaultgw(u32 *ip_addr , char mac[])
{
	int gw_index = 0; /* oif device index */
	struct net_device *gw_dev = NULL; /* oif device */

	route_dump(ip_addr, &gw_index);

	if (!(*ip_addr) || !gw_index) {
		/* RTW_INFO("No default GW\n"); */
		return -1;
	}

	gw_dev = dev_get_by_index(&init_net, gw_index);

	if (gw_dev == NULL) {
		/* RTW_INFO("get Oif Device Fail\n"); */
		return -1;
	}

	if (!arp_query(mac, *ip_addr, gw_dev)) {
		/* RTW_INFO( "arp query failed\n"); */
		dev_put(gw_dev);
		return -1;

	}
	dev_put(gw_dev);

	return 0;
}

int	rtw_gw_addr_query(_adapter *padapter)
{
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrctl = adapter_to_pwrctl(padapter);
	u32 gw_addr = 0; /* default gw address */
	unsigned char gw_mac[32] = {0}; /* default gw mac */
	int i;
	int res;

	res = get_defaultgw(&gw_addr, gw_mac);
	if (!res) {
		pmlmepriv->gw_ip[0] = gw_addr & 0xff;
		pmlmepriv->gw_ip[1] = (gw_addr & 0xff00) >> 8;
		pmlmepriv->gw_ip[2] = (gw_addr & 0xff0000) >> 16;
		pmlmepriv->gw_ip[3] = (gw_addr & 0xff000000) >> 24;
		_rtw_memcpy(pmlmepriv->gw_mac_addr, gw_mac, ETH_ALEN);
		RTW_INFO("%s Gateway Mac:\t" MAC_FMT "\n", __FUNCTION__, MAC_ARG(pmlmepriv->gw_mac_addr));
		RTW_INFO("%s Gateway IP:\t" IP_FMT "\n", __FUNCTION__, IP_ARG(pmlmepriv->gw_ip));
	} else
		RTW_INFO("Get Gateway IP/MAC fail!\n");

	return res;
}
#endif

void rtw_dev_unload(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctl = adapter_to_pwrctl(padapter);
	struct dvobj_priv *pobjpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &pobjpriv->drv_dbg;
	struct cmd_priv *pcmdpriv = &adapter_to_dvobj(padapter)->cmdpriv;

	if (padapter->bup == _TRUE) {
		RTW_INFO("==> "FUNC_ADPT_FMT"\n", FUNC_ADPT_ARG(padapter));

#ifdef CONFIG_WOWLAN
#ifdef CONFIG_GPIO_WAKEUP
		/*default wake up pin change to BT*/
		RTW_INFO("%s:default wake up pin change to BT\n", __FUNCTION__);
		rtw_hal_switch_gpio_wl_ctrl(padapter, pwrctl->wowlan_gpio_index, _FALSE);
#endif /* CONFIG_GPIO_WAKEUP */
#endif /* CONFIG_WOWLAN */

		rtw_set_drv_stopped(padapter);
#ifdef CONFIG_XMIT_ACK
		if (padapter->xmitpriv.ack_tx)
			rtw_ack_tx_done(&padapter->xmitpriv, RTW_SCTX_DONE_DRV_STOP);
#endif

		rtw_intf_stop(padapter);
		
		rtw_stop_drv_threads(padapter);

		if (ATOMIC_READ(&(pcmdpriv->cmdthd_running)) == _TRUE) {
			RTW_ERR("cmd_thread not stop !!\n");
			rtw_warn_on(1);
		}
		
		/* check the status of IPS */
		if (rtw_hal_check_ips_status(padapter) == _TRUE || pwrctl->rf_pwrstate == rf_off) { /* check HW status and SW state */
			RTW_PRINT("%s: driver in IPS-FWLPS\n", __func__);
			pdbgpriv->dbg_dev_unload_inIPS_cnt++;
		} else
			RTW_PRINT("%s: driver not in IPS\n", __func__);

		if (!rtw_is_surprise_removed(padapter)) {
#ifdef CONFIG_BT_COEXIST
			rtw_btcoex_IpsNotify(padapter, pwrctl->ips_mode_req);
#endif
#ifdef CONFIG_WOWLAN
			if (pwrctl->bSupportRemoteWakeup == _TRUE &&
			    pwrctl->wowlan_mode == _TRUE)
				RTW_PRINT("%s bSupportRemoteWakeup==_TRUE  do not run rtk_hal_deinit()\n", __FUNCTION__);
			else
#endif
			{
				/* amy modify 20120221 for power seq is different between driver open and ips */
				rtk_hal_deinit(padapter);
			}
			rtw_set_surprise_removed(padapter);
		}

		padapter->bup = _FALSE;

		RTW_INFO("<== "FUNC_ADPT_FMT"\n", FUNC_ADPT_ARG(padapter));
	} else {
		RTW_INFO("%s: bup==_FALSE\n", __FUNCTION__);
	}
	rtw_cancel_all_timer(padapter);
}

int rtw_suspend_free_assoc_resource(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &padapter->wdinfo;
#endif /* CONFIG_P2P */

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));

	if (rtw_chk_roam_flags(padapter, RTW_ROAM_ON_RESUME)) {
		if (check_fwstate(pmlmepriv, WIFI_STATION_STATE)
			&& check_fwstate(pmlmepriv, WIFI_ASOC_STATE)
			#ifdef CONFIG_P2P
			&& (rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)
				#if defined(CONFIG_IOCTL_CFG80211) && RTW_P2P_GROUP_INTERFACE
				|| rtw_p2p_chk_role(pwdinfo, P2P_ROLE_DEVICE)
				#endif
				)
			#endif /* CONFIG_P2P */
		) {
			RTW_INFO("%s %s(" MAC_FMT "), length:%d assoc_ssid.length:%d\n", __FUNCTION__,
				pmlmepriv->cur_network.network.Ssid.Ssid,
				MAC_ARG(pmlmepriv->cur_network.network.MacAddress),
				pmlmepriv->cur_network.network.Ssid.SsidLength,
				pmlmepriv->assoc_ssid.SsidLength);
			rtw_set_to_roam(padapter, 1);
		}
	}

	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) && check_fwstate(pmlmepriv, WIFI_ASOC_STATE)) {
		rtw_disassoc_cmd(padapter, 0, RTW_CMDF_DIRECTLY);
		/* s2-2.  indicate disconnect to os */
		rtw_indicate_disconnect(padapter, 0, _FALSE);
	}
#ifdef CONFIG_AP_MODE
	else if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter))
		rtw_sta_flush(padapter, _TRUE);
#endif

	/* s2-3. */
	rtw_free_assoc_resources(padapter, _TRUE);

	/* s2-4. */
	rtw_free_network_queue(padapter, _TRUE);

	if (check_fwstate(pmlmepriv, WIFI_UNDER_SURVEY)) {
		RTW_PRINT("%s: fw_under_survey\n", __func__);
		rtw_indicate_scan_done(padapter, 1);
		clr_fwstate(pmlmepriv, WIFI_UNDER_SURVEY);
	}

	if (check_fwstate(pmlmepriv, WIFI_UNDER_LINKING) == _TRUE) {
		RTW_PRINT("%s: fw_under_linking\n", __FUNCTION__);
		rtw_indicate_disconnect(padapter, 0, _FALSE);
	}

	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));
	return _SUCCESS;
}

#ifdef CONFIG_WOWLAN
int rtw_suspend_wow(_adapter *padapter)
{
	u8 ch, bw, offset;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct wowlan_ioctl_param poidparam;
	int ret = _SUCCESS;
	u8 en = _TRUE, i;
	struct registry_priv *registry_par = &padapter->registrypriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	_adapter *iface = NULL;
	struct hal_spec_t *hal_spec = GET_HAL_SPEC(padapter);

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));


	RTW_INFO("wowlan_mode: %d\n", pwrpriv->wowlan_mode);
	RTW_INFO("wowlan_pno_enable: %d\n", pwrpriv->wowlan_pno_enable);
#ifdef CONFIG_P2P_WOWLAN
	RTW_INFO("wowlan_p2p_enable: %d\n", pwrpriv->wowlan_p2p_enable);
#endif

	rtw_mi_netif_stop_queue(padapter);
	#ifdef CONFIG_CONCURRENT_MODE
	rtw_mi_buddy_netif_carrier_off(padapter);
	#endif

	/* 0. Power off LED */
	rtw_led_control(padapter, LED_CTL_POWER_OFF);

#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_GSPI_HCI)
	/* 2.only for SDIO disable interrupt */
	rtw_intf_stop(padapter);

	/* 2.1 clean interrupt */
	rtw_hal_clear_interrupt(padapter);
#endif /* CONFIG_SDIO_HCI */

	/* enable ac lifetime during scan to avoid txfifo not empty. */
	dvobj->lifetime_en = rtw_read8(padapter, 0x426);
	dvobj->pkt_lifetime = rtw_read32(padapter, 0x4c0);
	rtw_write8(padapter, 0x426, rtw_read8(padapter, 0x426) | 0x0f);
	if(hal_spec->tx_aclt_unit_factor == 1) {
		rtw_write16(padapter, 0x4c0, 0x1000);	// unit: 32us. 131ms
		rtw_write16(padapter, 0x4c0 + 2 , 0x1000);	// unit: 32us. 131ms
	} else {
		rtw_write16(padapter, 0x4c0, 0x0200);	// unit: 256us. 131ms
		rtw_write16(padapter, 0x4c0 + 2 , 0x0200);	// unit: 256us. 131ms
	}
	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if ((iface) && rtw_is_adapter_up(iface)) {
			rtw_write_port_cancel(iface);
			RTW_INFO(ADPT_FMT " write port cancel\n", ADPT_ARG(iface));
		}
	}
	RTW_INFO("lifetime_en=%x, pkt_lifetime=%x\n", rtw_read8(padapter, 0x426), rtw_read32(padapter, 0x4c0));
	rtw_msleep_os(200);

	/* 1. stop thread */
	rtw_set_drv_stopped(padapter);	/*for stop thread*/
	rtw_mi_stop_drv_threads(padapter);

	rtw_clr_drv_stopped(padapter);	/*for 32k command*/

	/* #ifdef CONFIG_LPS */
	/* rtw_set_ps_mode(padapter, PM_PS_MODE_ACTIVE, 0, 0, "WOWLAN"); */
	/* #endif */

	#ifdef CONFIG_SDIO_HCI
	/* 2.2 free irq */
	#if !(CONFIG_RTW_SDIO_KEEP_IRQ)
	sdio_free_irq(adapter_to_dvobj(padapter));
	#endif
	#endif/*CONFIG_SDIO_HCI*/

#ifdef CONFIG_RUNTIME_PORT_SWITCH
	if (rtw_port_switch_chk(padapter)) {
		RTW_INFO(" ### PORT SWITCH ###\n");
		rtw_hal_set_hwreg(padapter, HW_VAR_PORT_SWITCH, NULL);
	}
#endif
	if(registry_par->suspend_type == FW_IPS_WRC)
		rtw_hal_set_hwreg(padapter, HW_VAR_VENDOR_WOW_MODE, &en);
#ifdef CONFIG_LPS
	rtw_wow_lps_level_decide(padapter, _TRUE);
#endif
	poidparam.subcode = WOWLAN_ENABLE;
	rtw_hal_set_hwreg(padapter, HW_VAR_WOWLAN, (u8 *)&poidparam);
	if (rtw_chk_roam_flags(padapter, RTW_ROAM_ON_RESUME)) {
		if (check_fwstate(pmlmepriv, WIFI_STATION_STATE)
		    && check_fwstate(pmlmepriv, WIFI_ASOC_STATE)) {
			RTW_INFO("%s %s(" MAC_FMT "), length:%d assoc_ssid.length:%d\n", __FUNCTION__,
				pmlmepriv->cur_network.network.Ssid.Ssid,
				MAC_ARG(pmlmepriv->cur_network.network.MacAddress),
				pmlmepriv->cur_network.network.Ssid.SsidLength,
				 pmlmepriv->assoc_ssid.SsidLength);

			rtw_set_to_roam(padapter, 0);
		}
	}

	RTW_PRINT("%s: wowmode suspending\n", __func__);

	if (check_fwstate(pmlmepriv, WIFI_UNDER_SURVEY) == _TRUE) {
		RTW_PRINT("%s: fw_under_survey\n", __func__);
		rtw_indicate_scan_done(padapter, 1);
		clr_fwstate(pmlmepriv, WIFI_UNDER_SURVEY);
	}

	if (rtw_mi_check_status(padapter, MI_LINKED)) {
		ch =  rtw_mi_get_union_chan(padapter);
		bw = rtw_mi_get_union_bw(padapter);
		offset = rtw_mi_get_union_offset(padapter);
		RTW_INFO(FUNC_ADPT_FMT" back to linked/linking union - ch:%u, bw:%u, offset:%u\n",
			 FUNC_ADPT_ARG(padapter), ch, bw, offset);
		set_channel_bwmode(padapter, ch, offset, bw);
	}

#ifdef CONFIG_CONCURRENT_MODE
	rtw_mi_buddy_suspend_free_assoc_resource(padapter);
#endif

#ifdef CONFIG_BT_COEXIST
	rtw_btcoex_SuspendNotify(padapter, BTCOEX_SUSPEND_STATE_SUSPEND_KEEP_ANT);
#endif

	if (pwrpriv->wowlan_pno_enable) {
		RTW_PRINT("%s: pno: %d\n", __func__, pwrpriv->wowlan_pno_enable);
#ifndef RTW_HALMAC
#ifdef CONFIG_FWLPS_IN_IPS
		rtw_set_fw_in_ips_mode(padapter, _TRUE);
#endif
#else /* RTW_HALMAC */
		/* ICs with HALMAC that use NLO PS 32K no need to do anything */
#endif
	}
#ifdef CONFIG_LPS
	else {
		if(pwrpriv->wowlan_power_mgmt != PM_PS_MODE_ACTIVE) {
			rtw_set_ps_mode(padapter, pwrpriv->wowlan_power_mgmt, 0, 0, "WOWLAN");
		}
	}
#endif /* #ifdef CONFIG_LPS */

	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));
	return ret;
}
#endif /* #ifdef CONFIG_WOWLAN */

#ifdef CONFIG_AP_WOWLAN
int rtw_suspend_ap_wow(_adapter *padapter)
{
	u8 ch, bw, offset;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct wowlan_ioctl_param poidparam;
	int ret = _SUCCESS;

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));

	pwrpriv->wowlan_ap_mode = _TRUE;

	RTW_INFO("wowlan_ap_mode: %d\n", pwrpriv->wowlan_ap_mode);

	rtw_mi_netif_stop_queue(padapter);

	/* 0. Power off LED */
	rtw_led_control(padapter, LED_CTL_POWER_OFF);
#ifdef CONFIG_SDIO_HCI
	/* 2.only for SDIO disable interrupt*/
	rtw_intf_stop(padapter);

	/* 2.1 clean interrupt */
	rtw_hal_clear_interrupt(padapter);
#endif /* CONFIG_SDIO_HCI */

	/* 1. stop thread */
	rtw_set_drv_stopped(padapter);	/*for stop thread*/
	rtw_mi_stop_drv_threads(padapter);
	rtw_clr_drv_stopped(padapter);	/*for 32k command*/

	#ifdef CONFIG_SDIO_HCI
	/* 2.2 free irq */
	#if !(CONFIG_RTW_SDIO_KEEP_IRQ)
	sdio_free_irq(adapter_to_dvobj(padapter));
	#endif
	#endif/*CONFIG_SDIO_HCI*/

#ifdef CONFIG_RUNTIME_PORT_SWITCH
	if (rtw_port_switch_chk(padapter)) {
		RTW_INFO(" ### PORT SWITCH ###\n");
		rtw_hal_set_hwreg(padapter, HW_VAR_PORT_SWITCH, NULL);
	}
#endif

	rtw_wow_lps_level_decide(padapter, _TRUE);
	poidparam.subcode = WOWLAN_AP_ENABLE;
	rtw_hal_set_hwreg(padapter, HW_VAR_WOWLAN, (u8 *)&poidparam);

	RTW_PRINT("%s: wowmode suspending\n", __func__);

	if (rtw_mi_check_status(padapter, MI_LINKED)) {
		ch =  rtw_mi_get_union_chan(padapter);
		bw = rtw_mi_get_union_bw(padapter);
		offset = rtw_mi_get_union_offset(padapter);
		RTW_INFO("back to linked/linking union - ch:%u, bw:%u, offset:%u\n", ch, bw, offset);
		set_channel_bwmode(padapter, ch, offset, bw);
	}

	/*FOR ONE AP - TODO :Multi-AP*/
	{
		int i;
		_adapter *iface;
		struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			if ((iface) && rtw_is_adapter_up(iface)) {
				if (check_fwstate(&iface->mlmepriv, WIFI_AP_STATE | WIFI_MESH_STATE) == _FALSE)
					rtw_suspend_free_assoc_resource(iface);
			}
		}

	}

#ifdef CONFIG_BT_COEXIST
	rtw_btcoex_SuspendNotify(padapter, BTCOEX_SUSPEND_STATE_SUSPEND_KEEP_ANT);
#endif

#ifdef CONFIG_LPS
	if(pwrpriv->wowlan_power_mgmt != PM_PS_MODE_ACTIVE) {
		rtw_set_ps_mode(padapter, pwrpriv->wowlan_power_mgmt, 0, 0, "AP-WOWLAN");
	}
#endif

	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));
	return ret;
}
#endif /* CONFIG_AP_WOWLAN */


int rtw_suspend_normal(_adapter *padapter)
{
	int ret = _SUCCESS;

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));

#ifdef CONFIG_BT_COEXIST
	rtw_btcoex_SuspendNotify(padapter, BTCOEX_SUSPEND_STATE_SUSPEND);
#endif
	rtw_mi_netif_caroff_qstop(padapter);

	rtw_mi_suspend_free_assoc_resource(padapter);

	rtw_led_control(padapter, LED_CTL_POWER_OFF);

	if ((rtw_hal_check_ips_status(padapter) == _TRUE)
	    || (adapter_to_pwrctl(padapter)->rf_pwrstate == rf_off))
		RTW_PRINT("%s: ### ERROR #### driver in IPS ####ERROR###!!!\n", __FUNCTION__);


#ifdef CONFIG_CONCURRENT_MODE
	rtw_set_drv_stopped(padapter);	/*for stop thread*/
	rtw_stop_cmd_thread(padapter);
	rtw_drv_stop_vir_ifaces(adapter_to_dvobj(padapter));
#endif
	rtw_dev_unload(padapter);

	#ifdef CONFIG_SDIO_HCI
	sdio_deinit(adapter_to_dvobj(padapter));

	#if !(CONFIG_RTW_SDIO_KEEP_IRQ)
	sdio_free_irq(adapter_to_dvobj(padapter));
	#endif
	#endif /*CONFIG_SDIO_HCI*/

	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));
	return ret;
}

int rtw_suspend_common(_adapter *padapter)
{
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct debug_priv *pdbgpriv = &dvobj->drv_dbg;
	struct pwrctrl_priv *pwrpriv = dvobj_to_pwrctl(dvobj);
#ifdef CONFIG_WOWLAN
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct registry_priv *registry_par = &padapter->registrypriv;
#endif

	int ret = 0;
	systime start_time = rtw_get_current_time();

	RTW_PRINT(" suspend start\n");
	RTW_INFO("==> %s (%s:%d)\n", __FUNCTION__, current->comm, current->pid);

	pdbgpriv->dbg_suspend_cnt++;

	pwrpriv->bInSuspend = _TRUE;

	while (pwrpriv->bips_processing == _TRUE)
		rtw_msleep_os(1);

#ifdef CONFIG_IOL_READ_EFUSE_MAP
	if (!padapter->bup) {
		u8 bMacPwrCtrlOn = _FALSE;
		rtw_hal_get_hwreg(padapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn);
		if (bMacPwrCtrlOn)
			rtw_hal_power_off(padapter);
	}
#endif

	if ((!padapter->bup) || RTW_CANNOT_RUN(dvobj)) {
		RTW_INFO("%s bup=%d bDriverStopped=%s bSurpriseRemoved = %s\n", __func__
			 , padapter->bup
			 , dev_is_drv_stopped(dvobj) ? "True" : "False"
			, dev_is_surprise_removed(dvobj) ? "True" : "False");
		pdbgpriv->dbg_suspend_error_cnt++;
		goto exit;
	}
	rtw_mi_scan_abort(padapter, _TRUE);
	rtw_ps_deny(padapter, PS_DENY_SUSPEND);

	rtw_mi_cancel_all_timer(padapter);
	LeaveAllPowerSaveModeDirect(padapter);

	rtw_ps_deny_cancel(padapter, PS_DENY_SUSPEND);

	if (rtw_mi_check_status(padapter, MI_AP_MODE) == _FALSE) {
#ifdef CONFIG_WOWLAN
		if (WOWLAN_IS_STA_MIX_MODE(padapter))
			pwrpriv->wowlan_mode = _TRUE;
		else if ( registry_par->wowlan_enable && check_fwstate(pmlmepriv, WIFI_ASOC_STATE))
			pwrpriv->wowlan_mode = _TRUE;
		else if (pwrpriv->wowlan_pno_enable == _TRUE)
			pwrpriv->wowlan_mode |= pwrpriv->wowlan_pno_enable;

#ifdef CONFIG_P2P_WOWLAN
		if (!rtw_p2p_chk_state(&padapter->wdinfo, P2P_STATE_NONE) || P2P_ROLE_DISABLE != padapter->wdinfo.role)
			pwrpriv->wowlan_p2p_mode = _TRUE;
		if (_TRUE == pwrpriv->wowlan_p2p_mode)
			pwrpriv->wowlan_mode |= pwrpriv->wowlan_p2p_mode;
#endif /* CONFIG_P2P_WOWLAN */

		if (pwrpriv->wowlan_mode == _TRUE)
			rtw_suspend_wow(padapter);
		else
#endif /* CONFIG_WOWLAN */
			rtw_suspend_normal(padapter);
	} else if (rtw_mi_check_status(padapter, MI_AP_MODE)) {
#ifdef CONFIG_AP_WOWLAN
		rtw_suspend_ap_wow(padapter);
#else
		rtw_suspend_normal(padapter);
#endif /*CONFIG_AP_WOWLAN*/
	}


	RTW_PRINT("rtw suspend success in %d ms\n",
		  rtw_get_passing_time_ms(start_time));

exit:
	RTW_INFO("<===  %s return %d.............. in %dms\n", __FUNCTION__
		 , ret, rtw_get_passing_time_ms(start_time));

	return ret;
}

#ifdef CONFIG_WOWLAN
int rtw_resume_process_wow(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct dvobj_priv *psdpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;
	struct wowlan_ioctl_param poidparam;
	struct sta_info	*psta = NULL;
	struct registry_priv  *registry_par = &padapter->registrypriv;
	int ret = _SUCCESS;
	u8 en = _FALSE;

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));

	if (padapter) {
		pwrpriv = adapter_to_pwrctl(padapter);
	} else {
		pdbgpriv->dbg_resume_error_cnt++;
		ret = -1;
		goto exit;
	}

	if (RTW_CANNOT_RUN(psdpriv)) {
		RTW_INFO("%s pdapter %p bDriverStopped %s bSurpriseRemoved %s\n"
			 , __func__, padapter
			 , dev_is_drv_stopped(psdpriv) ? "True" : "False"
			, dev_is_surprise_removed(psdpriv) ? "True" : "False");
		goto exit;
	}

	pwrpriv->wowlan_in_resume = _TRUE;

	if (pwrpriv->wowlan_pno_enable) {
		RTW_PRINT("%s: pno: %d\n", __func__, pwrpriv->wowlan_pno_enable);
#ifndef RTW_HALMAC
#ifdef CONFIG_FWLPS_IN_IPS
		rtw_set_fw_in_ips_mode(padapter, _FALSE);
#endif
#else /* RTW_HALMAC */
		/* ToDo: ICs with HALMAC that use NLO PS 32K should leave LCLK here */
#endif
	} else {
#ifdef CONFIG_LPS
		if(pwrpriv->wowlan_power_mgmt != PM_PS_MODE_ACTIVE) {
			rtw_set_ps_mode(padapter, PM_PS_MODE_ACTIVE, 0, 0, "WOWLAN");
			rtw_wow_lps_level_decide(padapter, _FALSE);
		}
#endif /* CONFIG_LPS */
	}

	rtw_write8(padapter, 0x426, psdpriv->lifetime_en);
	rtw_write32(padapter, 0x4c0, psdpriv->pkt_lifetime);

	pwrpriv->bFwCurrentInPSMode = _FALSE;

#if defined(CONFIG_SDIO_HCI) || defined(CONFIG_PCI_HCI)
	rtw_mi_intf_stop(padapter);
	rtw_hal_clear_interrupt(padapter);
#endif

	#ifdef CONFIG_SDIO_HCI
	#if !(CONFIG_RTW_SDIO_KEEP_IRQ)
	if (sdio_alloc_irq(adapter_to_dvobj(padapter)) != _SUCCESS) {
		ret = -1;
		goto exit;
	}
	#endif
	#endif/*CONFIG_SDIO_HCI*/

	/* Disable WOW, set H2C command */
	poidparam.subcode = WOWLAN_DISABLE;
	rtw_hal_set_hwreg(padapter, HW_VAR_WOWLAN, (u8 *)&poidparam);

#ifdef CONFIG_CONCURRENT_MODE
	rtw_mi_buddy_reset_drv_sw(padapter);
#endif

	psta = rtw_get_stainfo(&padapter->stapriv, get_bssid(&padapter->mlmepriv));
	if (psta)
		set_sta_rate(padapter, psta);


	rtw_clr_drv_stopped(padapter);
	RTW_INFO("%s: wowmode resuming, DriverStopped:%s\n", __func__, rtw_is_drv_stopped(padapter) ? "True" : "False");

	if(registry_par->suspend_type == FW_IPS_WRC)
		rtw_hal_set_hwreg(padapter, HW_VAR_VENDOR_WOW_MODE, &en);

	rtw_mi_start_drv_threads(padapter);

	rtw_mi_intf_start(padapter);

	if(registry_par->suspend_type == FW_IPS_DISABLE_BBRF && !check_fwstate(pmlmepriv, WIFI_ASOC_STATE)) {
		if (!rtw_is_surprise_removed(padapter)) {
			rtk_hal_deinit(padapter);
			rtk_hal_init(padapter);
		}
		RTW_INFO("FW_IPS_DISABLE_BBRF hal deinit, hal init \n");
	}

#ifdef CONFIG_CONCURRENT_MODE
	rtw_mi_buddy_netif_carrier_on(padapter);
#endif

	/* start netif queue */
	rtw_mi_netif_wake_queue(padapter);

	if (padapter->pid[1] != 0) {
		RTW_INFO("pid[1]:%d\n", padapter->pid[1]);
		rtw_signal_process(padapter->pid[1], SIGUSR2);
	}

	if (rtw_chk_roam_flags(padapter, RTW_ROAM_ON_RESUME)) {
		if (pwrpriv->wowlan_wake_reason == FW_DECISION_DISCONNECT ||
		    pwrpriv->wowlan_wake_reason == RX_DISASSOC||
		    pwrpriv->wowlan_wake_reason == RX_DEAUTH) {

			RTW_INFO("%s: disconnect reason: %02x\n", __func__,
				 pwrpriv->wowlan_wake_reason);
			rtw_indicate_disconnect(padapter, 0, _FALSE);

			rtw_sta_media_status_rpt(padapter,
					 rtw_get_stainfo(&padapter->stapriv,
					 get_bssid(&padapter->mlmepriv)), 0);

			rtw_free_assoc_resources(padapter, _TRUE);
			pmlmeinfo->state = WIFI_FW_NULL_STATE;

		} else {
			RTW_INFO("%s: do roaming\n", __func__);
			rtw_roaming(padapter, NULL);
		}
	}

	if (pwrpriv->wowlan_mode == _TRUE) {
		pwrpriv->bips_processing = _FALSE;
		_set_timer(&adapter_to_dvobj(padapter)->dynamic_chk_timer, 2000);
#ifndef CONFIG_IPS_CHECK_IN_WD
		rtw_set_pwr_state_check_timer(pwrpriv);
#endif
	} else
		RTW_PRINT("do not reset timer\n");

	pwrpriv->wowlan_mode = _FALSE;

	/* Power On LED */
#ifdef CONFIG_RTW_SW_LED

	if (pwrpriv->wowlan_wake_reason == RX_DISASSOC||
	    pwrpriv->wowlan_wake_reason == RX_DEAUTH||
	    pwrpriv->wowlan_wake_reason == FW_DECISION_DISCONNECT)
		rtw_led_control(padapter, LED_CTL_NO_LINK);
	else
		rtw_led_control(padapter, LED_CTL_LINK);
#endif
	/* clean driver side wake up reason. */
	pwrpriv->wowlan_last_wake_reason = pwrpriv->wowlan_wake_reason;
	pwrpriv->wowlan_wake_reason = 0;

#ifdef CONFIG_BT_COEXIST
	rtw_btcoex_SuspendNotify(padapter, BTCOEX_SUSPEND_STATE_RESUME);
#endif /* CONFIG_BT_COEXIST */

exit:
	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));
	return ret;
}
#endif /* #ifdef CONFIG_WOWLAN */

#ifdef CONFIG_AP_WOWLAN
int rtw_resume_process_ap_wow(_adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct dvobj_priv *psdpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;
	struct wowlan_ioctl_param poidparam;
	struct sta_info	*psta = NULL;
	int ret = _SUCCESS;
	u8 ch, bw, offset;

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));

	if (padapter) {
		pwrpriv = adapter_to_pwrctl(padapter);
	} else {
		pdbgpriv->dbg_resume_error_cnt++;
		ret = -1;
		goto exit;
	}


#ifdef CONFIG_LPS
	if(pwrpriv->wowlan_power_mgmt != PM_PS_MODE_ACTIVE) {
		rtw_set_ps_mode(padapter, PM_PS_MODE_ACTIVE, 0, 0, "AP-WOWLAN");
		rtw_wow_lps_level_decide(padapter, _FALSE);
	}
#endif /* CONFIG_LPS */

	pwrpriv->bFwCurrentInPSMode = _FALSE;

	rtw_hal_disable_interrupt(padapter);

	rtw_hal_clear_interrupt(padapter);

	#ifdef CONFIG_SDIO_HCI
	#if !(CONFIG_RTW_SDIO_KEEP_IRQ)
	if (sdio_alloc_irq(adapter_to_dvobj(padapter)) != _SUCCESS) {
		ret = -1;
		goto exit;
	}
	#endif
	#endif/*CONFIG_SDIO_HCI*/
	/* Disable WOW, set H2C command */
	poidparam.subcode = WOWLAN_AP_DISABLE;
	rtw_hal_set_hwreg(padapter, HW_VAR_WOWLAN, (u8 *)&poidparam);
	pwrpriv->wowlan_ap_mode = _FALSE;

	rtw_clr_drv_stopped(padapter);
	RTW_INFO("%s: wowmode resuming, DriverStopped:%s\n", __func__, rtw_is_drv_stopped(padapter) ? "True" : "False");

	rtw_mi_start_drv_threads(padapter);

	if (rtw_mi_check_status(padapter, MI_LINKED)) {
		ch =  rtw_mi_get_union_chan(padapter);
		bw = rtw_mi_get_union_bw(padapter);
		offset = rtw_mi_get_union_offset(padapter);
		RTW_INFO(FUNC_ADPT_FMT" back to linked/linking union - ch:%u, bw:%u, offset:%u\n", FUNC_ADPT_ARG(padapter), ch, bw, offset);
		set_channel_bwmode(padapter, ch, offset, bw);
	}

	/*FOR ONE AP - TODO :Multi-AP*/
	{
		int i;
		_adapter *iface;
		struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			if ((iface) && rtw_is_adapter_up(iface)) {
				if (check_fwstate(&iface->mlmepriv, WIFI_AP_STATE | WIFI_MESH_STATE | WIFI_ASOC_STATE))
					rtw_reset_drv_sw(iface);
			}
		}

	}
	rtw_mi_intf_start(padapter);

	/* start netif queue */
	rtw_mi_netif_wake_queue(padapter);

	if (padapter->pid[1] != 0) {
		RTW_INFO("pid[1]:%d\n", padapter->pid[1]);
		rtw_signal_process(padapter->pid[1], SIGUSR2);
	}

#ifdef CONFIG_RESUME_IN_WORKQUEUE
	/* rtw_unlock_suspend(); */
#endif /* CONFIG_RESUME_IN_WORKQUEUE */

	pwrpriv->bips_processing = _FALSE;
	_set_timer(&adapter_to_dvobj(padapter)->dynamic_chk_timer, 2000);
#ifndef CONFIG_IPS_CHECK_IN_WD
	rtw_set_pwr_state_check_timer(pwrpriv);
#endif
	/* clean driver side wake up reason. */
	pwrpriv->wowlan_wake_reason = 0;

#ifdef CONFIG_BT_COEXIST
	rtw_btcoex_SuspendNotify(padapter, BTCOEX_SUSPEND_STATE_RESUME);
#endif /* CONFIG_BT_COEXIST */

	/* Power On LED */
#ifdef CONFIG_RTW_SW_LED

	rtw_led_control(padapter, LED_CTL_LINK);
#endif
exit:
	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));
	return ret;
}
#endif /* #ifdef CONFIG_APWOWLAN */

void rtw_mi_resume_process_normal(_adapter *padapter)
{
	int i;
	_adapter *iface;
	struct mlme_priv *pmlmepriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	for (i = 0; i < dvobj->iface_nums; i++) {
		iface = dvobj->padapters[i];
		if ((iface) && rtw_is_adapter_up(iface)) {
			pmlmepriv = &iface->mlmepriv;

			if (check_fwstate(pmlmepriv, WIFI_STATION_STATE)) {
				RTW_INFO(FUNC_ADPT_FMT" fwstate:0x%08x - WIFI_STATION_STATE\n", FUNC_ADPT_ARG(iface), get_fwstate(pmlmepriv));

				if (rtw_chk_roam_flags(iface, RTW_ROAM_ON_RESUME))
					rtw_roaming(iface, NULL);

			}
#ifdef CONFIG_AP_MODE
			else if (MLME_IS_AP(iface) || MLME_IS_MESH(iface)) {
				RTW_INFO(FUNC_ADPT_FMT" %s\n", FUNC_ADPT_ARG(iface), MLME_IS_AP(iface) ? "AP" : "MESH");
				rtw_ap_restore_network(iface);
			}
#endif
			else if (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE))
				RTW_INFO(FUNC_ADPT_FMT" fwstate:0x%08x - WIFI_ADHOC_STATE\n", FUNC_ADPT_ARG(iface), get_fwstate(pmlmepriv));
			else
				RTW_INFO(FUNC_ADPT_FMT" fwstate:0x%08x - ???\n", FUNC_ADPT_ARG(iface), get_fwstate(pmlmepriv));
		}
	}
}

int rtw_resume_process_normal(_adapter *padapter)
{
	struct net_device *pnetdev;
	struct pwrctrl_priv *pwrpriv;
	struct dvobj_priv *psdpriv;
	struct debug_priv *pdbgpriv;

	int ret = _SUCCESS;

	if (!padapter) {
		ret = -1;
		goto exit;
	}

	pnetdev = padapter->pnetdev;
	pwrpriv = adapter_to_pwrctl(padapter);
	psdpriv = padapter->dvobj;
	pdbgpriv = &psdpriv->drv_dbg;

	RTW_INFO("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));

	#ifdef CONFIG_SDIO_HCI
	/* interface init */
	if (sdio_init(adapter_to_dvobj(padapter)) != _SUCCESS) {
		ret = -1;
		goto exit;
	}
	#endif/*CONFIG_SDIO_HCI*/

	rtw_clr_surprise_removed(padapter);
	rtw_hal_disable_interrupt(padapter);

	#ifdef CONFIG_SDIO_HCI
	#if !(CONFIG_RTW_SDIO_KEEP_IRQ)
	if (sdio_alloc_irq(adapter_to_dvobj(padapter)) != _SUCCESS) {
		ret = -1;
		goto exit;
	}
	#endif
	#endif/*CONFIG_SDIO_HCI*/

	rtw_mi_reset_drv_sw(padapter);

	pwrpriv->bkeepfwalive = _FALSE;

	RTW_INFO("bkeepfwalive(%x)\n", pwrpriv->bkeepfwalive);
	if (pm_netdev_open(pnetdev, _TRUE) != 0) {
		ret = -1;
		pdbgpriv->dbg_resume_error_cnt++;
		goto exit;
	}

	rtw_mi_netif_caron_qstart(padapter);

	if (padapter->pid[1] != 0) {
		RTW_INFO("pid[1]:%d\n", padapter->pid[1]);
		rtw_signal_process(padapter->pid[1], SIGUSR2);
	}

#ifdef CONFIG_BT_COEXIST
	rtw_btcoex_SuspendNotify(padapter, BTCOEX_SUSPEND_STATE_RESUME);
#endif /* CONFIG_BT_COEXIST */

	rtw_mi_resume_process_normal(padapter);

#ifdef CONFIG_RESUME_IN_WORKQUEUE
	/* rtw_unlock_suspend(); */
#endif /* CONFIG_RESUME_IN_WORKQUEUE */
	RTW_INFO("<== "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));

exit:
	return ret;
}

int rtw_resume_common(_adapter *padapter)
{
	int ret = 0;
	systime start_time = rtw_get_current_time();
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);

	if (pwrpriv == NULL)
		return 0;

	if (pwrpriv->bInSuspend == _FALSE)
		return 0;

	RTW_PRINT("resume start\n");
	RTW_INFO("==> %s (%s:%d)\n", __FUNCTION__, current->comm, current->pid);

	if (rtw_mi_check_status(padapter, MI_AP_MODE) == _FALSE) {
#ifdef CONFIG_WOWLAN
		if (pwrpriv->wowlan_mode == _TRUE)
			rtw_resume_process_wow(padapter);
		else
#endif
			rtw_resume_process_normal(padapter);

	} else if (rtw_mi_check_status(padapter, MI_AP_MODE)) {
#ifdef CONFIG_AP_WOWLAN
		rtw_resume_process_ap_wow(padapter);
#else
		rtw_resume_process_normal(padapter);
#endif /* CONFIG_AP_WOWLAN */
	}

	pwrpriv->bInSuspend = _FALSE;
	pwrpriv->wowlan_in_resume = _FALSE;

	RTW_PRINT("%s:%d in %d ms\n", __FUNCTION__ , ret,
		  rtw_get_passing_time_ms(start_time));


	return ret;
}

#ifdef CONFIG_GPIO_API
u8 rtw_get_gpio(struct net_device *netdev, u8 gpio_num)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(netdev);
	return rtw_hal_get_gpio(adapter, gpio_num);
}
EXPORT_SYMBOL(rtw_get_gpio);

int  rtw_set_gpio_output_value(struct net_device *netdev, u8 gpio_num, bool isHigh)
{
	u8 direction = 0;
	u8 res = -1;
	_adapter *adapter = (_adapter *)rtw_netdev_priv(netdev);
	return rtw_hal_set_gpio_output_value(adapter, gpio_num, isHigh);
}
EXPORT_SYMBOL(rtw_set_gpio_output_value);

int rtw_config_gpio(struct net_device *netdev, u8 gpio_num, bool isOutput)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(netdev);
	return rtw_hal_config_gpio(adapter, gpio_num, isOutput);
}
EXPORT_SYMBOL(rtw_config_gpio);
int rtw_register_gpio_interrupt(struct net_device *netdev, int gpio_num, void(*callback)(u8 level))
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(netdev);
	return rtw_hal_register_gpio_interrupt(adapter, gpio_num, callback);
}
EXPORT_SYMBOL(rtw_register_gpio_interrupt);

int rtw_disable_gpio_interrupt(struct net_device *netdev, int gpio_num)
{
	_adapter *adapter = (_adapter *)rtw_netdev_priv(netdev);
	return rtw_hal_disable_gpio_interrupt(adapter, gpio_num);
}
EXPORT_SYMBOL(rtw_disable_gpio_interrupt);

#endif /* #ifdef CONFIG_GPIO_API */

#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE

int rtw_vendor_ie_get_api(struct net_device *dev, int ie_num, char *extra,
		u16 extra_len)
{
	int ret = 0;

	ret = rtw_vendor_ie_get_raw_data(dev, ie_num, extra, extra_len);
	return ret;
}
EXPORT_SYMBOL(rtw_vendor_ie_get_api);

int rtw_vendor_ie_set_api(struct net_device *dev, char *extra)
{
	return rtw_vendor_ie_set(dev, NULL, NULL, extra);
}
EXPORT_SYMBOL(rtw_vendor_ie_set_api);

#endif
