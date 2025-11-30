# 本地打包验证包含哪些文件
pio pkg pack

# 登录pio（可选）
pio account login

# 上传包
pio pkg publish --no-interactive

# 删除
pio pkg unpublish CommandUart@1.0.0

# 验证是否上传成功
pio access list
# https://registry.platformio.org/
pio pkg search CommandUart
