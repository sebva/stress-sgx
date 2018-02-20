FROM sebvaucher/sgx-base:sgx_2.1.1

COPY . .
RUN make SGX_MODE=HW SGX_DEBUG=0 SGX_PRERELEASE=1

ENTRYPOINT ["/entrypoint.sh", "./stress-ng"]
CMD ["--help"]

