FROM alpine:3.10

ENV ANSIBLE_VERSION=2.9.2

RUN apk add wget bash dos2unix
# RUN apk add python3 openssl ca-certificates git openssh sshpass yarn wget bash dos2unix \
#   && apk --update add --virtual build-dependencies \
#   python3-dev libffi-dev openssl-dev build-base \
#   && pip3 install --upgrade pip \
#   && pip3 install ansible==${ANSIBLE_VERSION} boto3 \
#   && apk del build-dependencies \
#   && rm -rf /var/cache/apk/*

# RUN set -xe \
#   && mkdir -p /etc/ansible \
#   && echo -e "[local]\nlocalhost ansible_connection=local" > \
#   /etc/ansible/hosts

ENV PROMETHEUS_VERSION 2.17.1
ENV PROMETHEUS_URL https://github.com/prometheus/prometheus/releases/download/v$PROMETHEUS_VERSION/prometheus-$PROMETHEUS_VERSION.linux-amd64.tar.gz
RUN wget -O /tmp/prometheus.tar.gz -q $PROMETHEUS_URL
RUN tar -C /tmp -xzf /tmp/prometheus.tar.gz
RUN cp /tmp/prometheus-$PROMETHEUS_VERSION.linux-amd64/prometheus "/bin/prometheus"
RUN mv /tmp/prometheus-$PROMETHEUS_VERSION.linux-amd64/ /etc/prometheus

COPY ./prometheus.yml /etc/prometheus/prometheus.yml
COPY ./entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
RUN dos2unix /entrypoint.sh

# COPY ./rule-generator /rule-generator

ENTRYPOINT [ "/entrypoint.sh" ]
