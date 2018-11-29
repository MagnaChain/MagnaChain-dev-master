Binaries for MagnaChain version 0.3.21 are available at:
  https://sourceforge.net/projects/magnachain/files/MagnaChain/magnachain-0.3.21/

Changes and new features from the 0.3.20 release include:

* Universal Plug and Play support.  Enable automatic opening of a port for incoming connections by running magnachain or magnachaind with the - -upnp=1 command line switch or using the Options dialog box.

* Support for full-precision magnachain amounts.  You can now send, and magnachain will display, magnachain amounts smaller than 0.01.  However, sending fewer than 0.01 magnachains still requires a 0.01 magnachain fee (so you can send 1.0001 magnachains without a fee, but you will be asked to pay a fee if you try to send 0.0001).

* A new method of finding magnachain nodes to connect with, via DNS A records. Use the -dnsseed option to enable.

For developers, changes to magnachain's remote-procedure-call API:

* New rpc command "sendmany" to send magnachains to more than one address in a single transaction.

* Several bug fixes, including a serious intermittent bug that would sometimes cause magnachaind to stop accepting rpc requests. 

* -logtimestamps option, to add a timestamp to each line in debug.log.

* Immature blocks (newly generated, under 120 confirmations) are now shown in listtransactions.
