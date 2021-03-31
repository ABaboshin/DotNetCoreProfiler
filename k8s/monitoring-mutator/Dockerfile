FROM golang:1.16.2-alpine AS build
ENV GO111MODULE on
ENV CGO_ENABLED 0

RUN apk add git make openssl

WORKDIR /go/src/github.com/ABaboshin/k8s/monitoring-mutator
ADD . .
RUN go get -v ./...
RUN go build -v -o monitoring-mutator main.go

FROM scratch
WORKDIR /app
COPY --from=build /etc/ssl/certs/ca-certificates.crt /etc/ssl/certs/
COPY --from=build /go/src/github.com/ABaboshin/k8s/monitoring-mutator/monitoring-mutator .
CMD ["/app/monitoring-mutator"]

VOLUME [ "/etc/webhook/certs/" ]
