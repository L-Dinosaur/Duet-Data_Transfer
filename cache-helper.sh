for i in {1..3}
do
	cat /home/dino/linux-image-4.6.0+duet-a23a4a5_4.6.0+duet-a23a4a5-10.00.Custom_amd64.deb > /dev/null
	sleep 5s
	echo 3 > /proc/sys/vm/drop_caches
	sleep 5s
done

