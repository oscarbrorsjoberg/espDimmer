FROM debian:latest

RUN apt-get update && \
    apt-get install -y \
        openssh-client \
        git \
        wget \
        libgd-dev \
        libxml2 \
        libxslt1-dev \
        libpcre3 \
        libpcre3-dev \
        zlib1g \
        zlib1g-dev \
        openssl \
        libssl-dev \
        libtool \
        automake \
        gcc \
        g++ \
        make && \
    rm -rf /var/cache/apt

ENV NGINX_USER="nginx" \
    NGINXR_UID="8987" \
    NGINX_GROUP="nginxers" \
    NGINX_GID="1024"  

RUN groupadd -r --gid "${NGINX_GID}" "${NGINX_GROUP}"
RUN useradd -r --uid "${NGINXR_UID}" --gid "${NGINX_GID}" "${NGINX_USER}"

RUN mkdir -p /src/
RUN mkdir -p /nginx/

RUN chown -R "${NGINX_USER}":"${NGINX_GROUP}" /src/
RUN chmod -R 755 /src/


RUN wget "http://nginx.org/download/nginx-1.25.2.tar.gz" && \
    tar -C /usr/src -xzvf nginx-1.25.2.tar.gz

RUN mkdir -p -m 0600 ~/.ssh && \
    ssh-keyscan github.com >> ~/.ssh/known_hosts

WORKDIR /src/ngx_devel_kit
RUN --mount=type=ssh git clone git@github.com:simpl/ngx_devel_kit .

WORKDIR /src/set-misc-nginx-module
RUN --mount=type=ssh git clone git@github.com:openresty/set-misc-nginx-module.git .

WORKDIR /usr/src/nginx-1.25.2
RUN ./configure --user=nginx --sbin-path=/usr/bin/nginx --pid-path=/tmp/nginx.pid \ 
                --with-threads --prefix=/nginx \
                --with-http_v2_module \
                --with-pcre --with-compat --with-http_ssl_module \
                --add-dynamic-module=/src/ngx_devel_kit \ 
                --add-dynamic-module=/src/set-misc-nginx-module \
                --with-http_image_filter_module=dynamic \ 
                --modules-path=/nginx/modules && \
    make && make modules && make install

RUN chown -R "${NGINX_USER}":"${NGINX_GROUP}" /nginx/
RUN chmod -R 755 /nginx/

WORKDIR /nginx
USER nginx
