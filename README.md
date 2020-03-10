# rdr_port
redirect port in N seconds. 短时的重定向端口。

--------
Execute `rdr_port_ipt` or  `rdr_port_nft` :
* Insert a port redirection rule into the iptables rule table or nftable chain according to the source IP provided.
* The program resides in the background and waits for N seconds (depending on the command line parameters).
* Remove the previously inserted redirection rule from the iptables or nftable.
* The program done & exit.

The compiled executable file can be set with suid (chmod 4555 ...), which is called and executed by web script and temporarily redirected the port.   
For example 443 -> 22, You can use the SSH client connect to port 443 in a few seconds, and then even if the redirection rule is deleted, your SSH connection will not be affected. Until you logout.  

For example, PHP:
 ```
 $ip='1.2.3.4';
 exec('/somepath/rdr_port_ipt  '.$ip.'  20  a  b');
```

-------
执行 `rdr_port_ipt` ：
* 根据 提供的来源 IP，在 iptables 规则表, 或nftable chain 中插入一条端口重定向规则。
* 程序驻留后台等待 N 秒(根据命令行参数)。
* 从 iptables 规则中 或nftable chain中 删除之前插入的重定向规则。
* 程序结束退出。

编译后的可执行文件，可以设置SUID (chmod 4555 ...)，由网页脚本调用，达到临时重定向端口的目的。  
如 443 -> 22 , 你可以在几秒内用 SSH 客户端连接443口，然后，即使重定向规则被删除，也不影响你的 SSH 连接。直到你主动logout。  

比如，php：
```
 $ip='1.2.3.4';
 exec('/somepath/rdr_port_ipt  '.$ip.'  20  a  b');
```

-------

2020-03-10.   
end.
