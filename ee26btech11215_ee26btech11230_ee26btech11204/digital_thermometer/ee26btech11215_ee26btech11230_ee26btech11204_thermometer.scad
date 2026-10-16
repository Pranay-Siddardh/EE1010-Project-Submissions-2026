$fn=64;

translate([0,-18,0]){
//base
translate([0,30,0]){
cube([90,80
,2]);

//Upwall
rotate([0,0,90])
translate([0,-180,0])
difference(){
cube([86,76
,2]);
translate([6,44,-1])
cube([74,26,4]);
translate([3,3,1])
cylinder(d=2,h=4,center=true);
translate([3,73,1])
cylinder(d=2,h=4,center=true);
translate([83,73,1])
cylinder(d=2,h=4,center=true);
translate([83,3,1])
cylinder(d=2,h=4,center=true);
translate([72,12,1])
cylinder(d=6,h=4,center=true);
}

//lateral-sides
translate([0,0,2])
cube([90,2
,30]);

translate([0,78,2])
cube([90,2
,30]);

translate([0,2,2])
difference(){
cube([2,78
,30]);
translate([-1,4,4])
cube([4,12,14]);
translate([-1,38,4])
cube([4,14,14]);
}
    
translate([88,2,2])
difference(){
cube([2,76
,30]);
translate([-1,64,24])
cube([4,4,7]);
}

translate([5,5,27])
difference(){
cube([6,6,6],center=true);
translate([0,0,0])
cylinder(d=2,h=8,center=true);
}

translate([5,75,27])
difference(){
cube([6,6,6],center=true);
translate([0,0,0])
cylinder(d=2,h=8,center=true);
}

translate([85,75,27])
difference(){
cube([6,6,6],center=true);
translate([0,0,0])
cylinder(d=2,h=8,center=true);
}

translate([85,5,27])
difference(){
cube([6,6,6],center=true);
translate([0,0,0])
cylinder(d=2,h=8,center=true);
}
translate([24,30,2])
cylinder(d=4,h=4);
translate([24,50,2])
cylinder(d=4,h=4);
translate([60,30,2])
cylinder(d=4,h=4);
translate([60,50,2])
cylinder(d=4,h=4);

translate([60,84,10])
rotate([180,-90,0])
difference() {
    cylinder(r = 6, h = 14);
    translate([0, 0, -0.5]) 
        cylinder(r = 4, h = 15);
    translate([0, 0, -0.5]) 
        cube([7, 7,15]);
}

translate([20,84,10])
rotate([180,-90,0])
difference() {
    cylinder(r = 6, h = 14);
    translate([0, 0, -0.5]) 
        cylinder(r = 4, h = 15);
    translate([0, 0, -0.5]) 
        cube([7, 7,15]);
}
}

translate([26,20,16])
cube([40,10,2]);
translate([16,18,7])
cube([60,2,20]);
}


translate([110,110,0])
cube([40,26,2]);
translate([110,134,2])
cube([40,2,12]);
translate([110,110,2])
cube([40,2,12]);
