```bash

helm repo add prometheus-community https://prometheus-community.github.io/helm-charts

helm install statsd prometheus-community/prometheus-statsd-exporter

helm upgrade --install monitoring-mutator ../charts/monitoring-mutator -f minikube-mutator.yml

& minikube -p minikube docker-env | Invoke-Expression

docker build -f Dockerfile -t sample:app ../../
docker build -f Dockerfile.interceptor -t sample:init ../../

kubectl apply -f test.yml
```
