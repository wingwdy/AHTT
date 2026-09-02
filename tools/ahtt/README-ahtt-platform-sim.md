# AHTT 心跳超时模拟服务

## 用途

`ahtt_platform_sim.py` 是独立的 TCP 平台模拟服务，用于在真实安徽铁塔平台无法控制时验证M2心跳超时。

- 收到设备`0x01`签到后，回复合法的同命令、零参数应答。
- 默认收到设备`0x81`心跳时仅打印原始帧，**不回复且不关闭TCP连接**。
- 使用`--reply-heartbeat`时才回复`0x81`，仅用于正常闭环冒烟测试。

## 本地自检

```powershell
python tools\ahtt\test_ahtt_platform_sim.py
python tools\ahtt\ahtt_platform_sim.py --selftest
```

预期：4个单元测试通过，且输出`AHTT platform simulator selftest: PASS`。

## 启动静默服务

```powershell
python tools\ahtt\ahtt_platform_sim.py --host 0.0.0.0 --port 18888
```

未传`--reply-heartbeat`即为心跳静默模式。服务会打印：

```text
[RX] cmd=0x01 ...
[TX] cmd=0x01 ...
[RX] cmd=0x81 ...
[SIM] heartbeat reply intentionally suppressed; TCP remains connected
```

## 板端使用

1. 将服务部署到4G设备能访问的公网IP或已做好端口映射的地址；本机`192.168.x.x`地址通常不能被4G设备直接访问。
2. 在测试设备上临时配置模拟服务地址和端口，例如：

   ```text
   set para domain:你的公网域名,18888
   ```

3. 重连后确认模拟服务收到`0x01`并回复；随后持续保存至少35秒设备串口日志。
4. 预期设备在约0秒、10秒、20秒各发送一次`0x81`，约30秒进入Offline并关闭TCP。
5. 测试完成后必须恢复生产地址：

   ```text
   set para domain:www.ahttcd.cn,8888
   ```

## 安全边界

- 此服务不转发生产平台数据，也不发送任何充电、设置或升级命令。
- 服务仅用于指定测试设备；端口暴露到公网时应限制测试时段并在完成后关闭端口映射。
- 不要用断网替代本测试；断网验证的是网络层，不是`0x81`命令级超时。
