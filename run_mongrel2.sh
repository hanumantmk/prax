#!/bin/sh

m2sh load -config mongrel2.conf

mkdir -p run logs tmp

m2sh start -name test
