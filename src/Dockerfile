FROM ubuntu:18.04
LABEL maintainer=MagnaChain \
    version=0.0.1 \
    desp='MagnaChain Docker image builder'
#工作目录
WORKDIR /root
#定义区块数据挂载点和环境变量
ENV CHAIN_DATA=/root/blocks/
ENV APP=/root/app
ENV PATH ${APP}:$PATH
#暴露节点端口
EXPOSE 8332
#复制所需的依赖库和可执行文件
COPY ./app /root/app/
COPY docker-entrypoint.sh /usr/local/bin/
#设置可执行权限，并且建立区块数据存放目录，main为主链数据，side为侧链数据
#添加so库的查找路径
RUN chmod +x ${APP}/magnachaind ${APP}/magnachain-cli /usr/local/bin/docker-entrypoint.sh \
    &&  mkdir -p blocks/main blocks/side \
    && echo ${APP} >> /etc/ld.so.conf.d/app.conf \
    && ldconfig
#TODO HEALTHCHECK健康检查
# HEALTHCHECK    
#启动节点
ENTRYPOINT [ "docker-entrypoint.sh","magnachaind"]
CMD [ "-printtoconsole", "-datadir=/root/blocks/main" ]






