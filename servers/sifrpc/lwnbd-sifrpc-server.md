lwNBD-sifrpc-server(1) -- Playstation 2 SIF server
=============================================

TARGETS : PlayStation 2

STATUS : experimental

```mermaid
sequenceDiagram
EE->>IOP: SifCallRPC()
IOP->>IOP: Process
IOP-->>EE
```
Currently, it only contains essential RPC to control the lwnbd irx from EE. The strategy here is to gradually build up the glue as you need it.

The final purpose of this plugin is to provide a way to speak transparently between EE and IOP without using SIF wrappers, but using lwnbd_plugin_t interface and other lwnbd mecanism. The advantage of such a mechanism is that it avoids the systematic addition of an SIF wrapper for any IOP-side library. These wrappers are often not synchronised with the IOP, are of variable quality and tend to develop functionalities that go beyond their role. This plugin avoids having to redo drivers on the EE side to switch the TCP server to the EE, which is more efficient in terms of transfer speed, and allows you to benefit from the power of the EE to manage compression, as in the case of TLS support. So it is indeed a wrapper for lwnbd, but it's kind of the final wrapper.

# The Library Hierarchy 

The libraries that use the SIF to transfer data are arranged in a hierarchy : 

* RPC
* CMD
* DMA
* [DMAC]

# TODO 

* asyncronous using sceSifCheckStatRpc() SifGetNextRequest / SifExecRequest

# see also

* [TIF](https://www.psx-place.com/threads/retail-debugging-startup-card.14027/)
* [eRPC](https://github.com/EmbeddedRPC/erpc)
