#!/bin/sh
#
#   Bacula® - The Network Backup Solution
#
# Copyright (C) 2000-2020 Kern Sibbald
# License: BSD 2-Clause; see file LICENSE-FOSS
#
#   Bacula(R) is a registered trademark of Kern Sibbald.
#
# This is a Bacula archive tool for backup/restore files on Docker volumes.
# Author: Radosław Korzeniewski, radekk@inteos.pl, Inteos Sp. z o.o.
#
FROM busybox:latest
LABEL maintainer="Radosław Korzeniewski <radekk@inteos.pl>"
LABEL org.label-schema.schema-version="1.0"
LABEL org.label-schema.description="This is a Bacula container for backup/restore Docker volumes using archive tools."
LABEL org.label-schema.vendor="Bacula Systems S.A."
LABEL org.label-schema.version="1.2"
COPY baculatar /baculatar
COPY tar /tar
CMD ["/baculatar", "backup"]
ENTRYPOINT ["/baculatar"]
