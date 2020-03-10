This file is for iptables.

--------
Compile with `gcc -O -o rdr_port_ipt  rdr_port_ipt.c` .

Execute `rdr_port_ipt` :
* Insert a port redirection rule in the iptables rule table according to the source IP provided.
  * Like this: `iptables -t nat -A PREROUTING -s 1.2.3.4 -p tcp --dport 443 -j REDIRECT --to 22`
* The program resides in the background and waits for N seconds (depending on the command line parameters).
* Remove the previously inserted redirection rule from the iptables rule.
  * Like this: `iptables -t nat -D PREROUTING -s 1.2.3.4 -p tcp --dport 443 -j REDIRECT --to 22`
* The program done & exit.

-------
用 `gcc -O -o rdr_port_ipt  rdr_port_ipt.c` 编译

执行 `rdr_port_ipt` ：
* 根据 提供的来源 IP，在 iptables 规则表中插入一条端口重定向规则。
  * 例如: `iptables -t nat -A PREROUTING -s 1.2.3.4 -p tcp --dport 443 -j REDIRECT --to 22`
* 程序驻留后台等待 N 秒(根据命令行参数)。
* 从 iptables 规则中删除之前插入的重定向规则。
  * 例如: `iptables -t nat -D PREROUTING -s 1.2.3.4 -p tcp --dport 443 -j REDIRECT --to 22`
* 程序结束退出。

-------
2020-03-10.   
end.
