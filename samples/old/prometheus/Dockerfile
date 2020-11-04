FROM alpine:3.10

ENV ANSIBLE_VERSION=2.9.2

RUN apk add wget bash dos2unix

ENV PROMETHEUS_VERSION 2.22.0
ENV PROMETHEUS_URL https://github.com/prometheus/prometheus/releases/download/v$PROMETHEUS_VERSION/prometheus-$PROMETHEUS_VERSION.linux-amd64.tar.gz
RUN wget -O /tmp/prometheus.tar.gz -q $PROMETHEUS_URL
RUN tar -C /tmp -xzf /tmp/prometheus.tar.gz
RUN cp /tmp/prometheus-$PROMETHEUS_VERSION.linux-amd64/prometheus "/bin/prometheus"
RUN mv /tmp/prometheus-$PROMETHEUS_VERSION.linux-amd64/ /etc/prometheus

COPY ./prometheus.yml /etc/prometheus/prometheus.yml
COPY alert-rules.yml /etc/prometheus/alert-rules.yml
COPY ./entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh
RUN dos2unix /entrypoint.sh

# COPY ./rule-generator /rule-generator

ENTRYPOINT [ "/entrypoint.sh" ]
