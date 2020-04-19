#!/bin/bash

#cd /rule-generator
#ansible-playbook rule-generator.yml
#cp alert-rules.yml /etc/prometheus/alert-rules.yml

/bin/prometheus $@
