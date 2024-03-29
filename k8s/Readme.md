# Monitoring Mutator

A Mutating admission webhook which injects tracing and monitoring using the .net core profiler from this repository.

Features:
- auto detecting of the app names based on the parent deployment/statefulset name,
- inject custom profiler configuration via configmap (see Sample).

## Implementation

See [monitoring-mutator](monitoring-mutator).

## Helm chart

A helm chart can be found in [charts/monitoring-mutator](charts/monitoring-mutator).

A custom ceritificate signer is using https://github.com/ABaboshin/kubernetes-cert-signer to manage the certificates.

| Parameter                          | Description                                                                                                                                                                                                                                               | Default                                         |
|------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|-------------------------------------------------|
| `init.image`          | An image for the init container to sign the certificates                                                                                                                    | `ababoshin/kubernetes-cert-signer:20210401`                        |
| `init.imagePullPolicy`          | An init image pull policy                                                                                                                    | `Always`                        |
| `mutator.image`          | An image for the mutator container                                                                                                                    | `ababoshin/monitoring-mutator`                        |
| `mutator.imagePullPolicy`          | An mutator image pull policy                                                                                                                    | `Always`                        |
| `config`          | A json configuration with environments variables, volumes and init containers for the monitorable containers. See sample for details                                                                                                                    | `Always`                        |
| `namespaceSelector`          | A namespace selector for the webhook                                                                                                                    | `{}`                        |
| `objectSelector`          | An oject (pod) selector for the webhook                                                                                                                    | `{}`                        |


## Sample


See [monitoring-mutator](monitoring-mutator).

To run the sample execute

```bash

cd sample
# install a statsd exporter
helm repo add prometheus-community https://prometheus-community.github.io/helm-charts
helm install statsd prometheus-community/prometheus-statsd-exporter -n test --create-namespace

# install the monitoring mutator
helm upgrade --install monitoring-mutator ../charts/monitoring-mutator -f sample-mutator.yml

# start and connect a minikube instance
& minikube -p minikube docker-env | Invoke-Expression

# build a sample app
docker build -f Dockerfile -t sample:app ../../

# build a sample interceptors
docker build -f Dockerfile.interceptor -t sample:init ../../

# deploy the app
kubectl apply -f test.yml

# Call http://app1:5000/api/sample from inside the cluster
# Check that statsd has the new metrics which container service names like app1 and app2.
```

