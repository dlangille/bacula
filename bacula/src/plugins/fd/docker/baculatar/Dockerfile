#
#   Bacula® - The Network Backup Solution
#
#   Copyright (C) 2007-2017 Bacula Systems SA
#   All rights reserved.
#
#   The main author of Bacula is Kern Sibbald, with contributions from many
#   others, a complete list can be found in the file AUTHORS.
#
#   Licensees holding a valid Bacula Systems SA license may use this file
#   and others of this release in accordance with the proprietary license
#   agreement provided in the LICENSE file.  Redistribution of any part of
#   this release is not permitted.
#
#   Bacula® is a registered trademark of Kern Sibbald.
#
#
# Copyright (c) 2019 by Inteos sp. z o.o.
# All rights reserved. IP transfered to Bacula Systems according to agreement.
#
# This is a Bacula container for backup/restore Docker volumes using archive tools.
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
