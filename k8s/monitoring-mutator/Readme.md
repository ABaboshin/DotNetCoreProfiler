# Monitoring Mutator

## Build an image

```bash
bash build-image.sh
```

## Deploy

```
helm upgrade --install monitoring-mutator ../charts/monitoring-mutator -f values.yml
```
