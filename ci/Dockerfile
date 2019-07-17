FROM ubuntu:19.04

RUN apt-get update \
    && apt-get install -y -qq --no-install-recommends \
        cmake \
        g++ \
        make \
        qt5-default \
        qtbase5-dev \
        qttools5-dev 


ENTRYPOINT ["/bin/bash"]
