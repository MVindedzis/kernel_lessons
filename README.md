Hey there!

Well this is my very first "practical" kernel module on raspberry pi 4.

I will be honest, i used AI tools to help me with this, no way i could make this out of thin air, but my goal is to learn and master this craft.

anyways, i had some troubles initializing pwm hardware on my raspberry pi, so i though.. I can just bit bang gpio and so i chose hrtimer for this.

I had to use spinlocks to update variables, because my servo did something funny, other processes may have messed with the timer, causing the thing to jiggle.

i created it (with the help of AI) few months ago and it was hell of a project.
lots of fun solving kernel panics.

Short story long, i currently put down my kernel studies and im focused on reinforcing my C knowledge. 
