FROM grafana/grafana:7.0.0-beta1

USER root

ENV ANSIBLE_VERSION=2.9.2

RUN apk add python3 openssl ca-certificates git openssh sshpass yarn wget bash dos2unix \
  && apk --update add --virtual build-dependencies \
  python3-dev libffi-dev openssl-dev build-base \
  && pip3 install --upgrade pip \
  && pip3 install ansible==${ANSIBLE_VERSION} boto3 \
  && apk del build-dependencies \
  && rm -rf /var/cache/apk/*

RUN set -xe \
  && mkdir -p /etc/ansible \
  && echo -e "[local]\nlocalhost ansible_connection=local" > \
  /etc/ansible/hosts

COPY datasources/* /etc/grafana/provisioning/datasources/
COPY dashboards/* /etc/grafana/provisioning/dashboards/

EXPOSE 3000
