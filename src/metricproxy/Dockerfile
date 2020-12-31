FROM golang:alpine3.11 as builder

RUN apk update && apk add --no-cache git
WORKDIR $GOPATH/src/metricproxy
COPY src/metricproxy .
RUN go get -d -v

RUN GOOS=linux GOARCH=amd64 CGO_ENABLED=0 go build -ldflags="-w -s" -o /metricproxy

FROM scratch

COPY --from=builder /metricproxy /metricproxy

ENTRYPOINT  [ "/metricproxy" ]
