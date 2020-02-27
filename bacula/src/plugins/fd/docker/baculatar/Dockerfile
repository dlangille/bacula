#!/bin/sh
#
#   Bacula® - The Network Backup Solution
#
#   Copyright (C) 2000-2020 Kern Sibbald
#
#   The original author of Bacula is Kern Sibbald, with contributions
#   from many others, a complete list can be found in the file AUTHORS.
#
#   You may use this file and others of this release according to the
#   license defined in the LICENSE file, which includes the Affero General
#   Public License, v3.0 ("AGPLv3") and some additional permissions and
#   terms pursuant to its AGPLv3 Section 7.
#
#   This notice must be preserved when any source code is
#   conveyed and/or propagated.
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
