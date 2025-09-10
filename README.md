Hey there!

Well this is my very first "practical" kernel module on raspberry pi 4.
It uses user-space app servo_controller.c to send key presses via write() syscall.
I know it runs on hope and prayer and it can be made better.

Anyways, i had some troubles initializing pwm hardware on my raspberry pi, so i though.. I can just bit bang gpio and so i chose hrtimer for this.

I had to use spinlocks to update variables, because my servo did something funny, other processes may have messed with the timer, causing the thing to jiggle at random intervals.

I created it (with the help of AI) few months ago and it was hell of a project.
lots of fun solving kernel panics.

