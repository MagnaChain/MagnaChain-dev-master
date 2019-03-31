#!/usr/bin/perl -w
use 5.010;
use File::Copy;
use Cwd;
# 查找可执行程序依赖，并复制到指定目录,默认是当前目录
# usage:
# chmod + cp_ldd_so.pl
# ldd app|./cp_ldd_so.pl [ dest ]

my $script = $ARGV[0];
my $times = $ARGV[1];
say($script);
for (1 .. $times) {
	# body...
	say $_;
	system "python3 test_runner.py $script";
	sleep 30;
	`killall -9 magnachaind`;
	`killall -9 test_runner.py`;
	say '*' x 30;
	sleep 30;
}