package main

import (
	"fmt"
	"net"
	"strconv"
)

func ExtractIPInfo(addr string) (*net.IPAddr, int, error) {
	host, portStr, err := net.SplitHostPort(addr)

	if err != nil {
		return nil, 0, fmt.Errorf("bad listening address: %s", addr)
	}

	if host == "" {
		host = "0.0.0.0"
	}

	ip, err := net.ResolveIPAddr("ip", host)
	if err != nil {
		return nil, 0, fmt.Errorf("Unable to resolve %s: %s", host, err)
	}

	port, err := strconv.Atoi(portStr)
	if err != nil || port < 0 || port > 65535 {
		return nil, 0, fmt.Errorf("Bad port %s: %s", portStr, err)
	}

	return ip, port, nil
}

func filter(ss []string, test func(string) bool) (ret []string) {
	for _, s := range ss {
		if test(s) {
			ret = append(ret, s)
		}
	}
	return
}
