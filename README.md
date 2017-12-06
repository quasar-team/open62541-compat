# open62541-compat
Adapts open62541 API to UA Toolkit API for usage in (1)Quasar-based projects, (2) independent UASDK-based projects

For bugs or suggestions please file a GitHub ticket.

Quick-start guide in Quasar
---------------------------
The procedure is documented in Quasar project.
In Quasar repo, read:

Documentation/AlternativeBackends.html

Quick-start guide to get an stand-alone(independent) library
-----------------------------------------------------------
1. clone the repo

2. run build_standalone.py

###### How do you use it in your independent UASDK server or client? ######

The include directory contains UASDK-(partially)-compatible headers, and you should link with the following libs:
* build/libopen62541-compat.a
* open62541/build/libopen62541.a

For questions/issues: file a ticket or write to piotr.nikiel@cern.ch

