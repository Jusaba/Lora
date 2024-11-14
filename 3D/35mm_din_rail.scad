//Length of DIN Rail to print (mm)
length = 50;//[30:200]

//Number of holes on the rail
holes_per_segment = 2; //[0:10]

//Diameter of mounting holes (mm)
hole_diameter = 3; //[0.5:5]

//Include Male Key (for attaching to other pieces of rail)
male_key = 1; // [0:1]

//Include Female Key (for attaching to other pieces of rail)
female_key = 1; //[0:1]



module punch_mounting_holes(adjustment)
{
	echo(adjustment);
	for(j = [1:holes_per_segment])
	{
		translate([17.5,8,(j+adjustment)*length/(holes_per_segment + 1)]) 
			rotate([90,0,0]) 
				cylinder(r=hole_diameter/2, h=3, $fn=8);
	}
}


difference()
{
	difference()
	{
		union()
		{
			linear_extrude(length)import_dxf("PerfilRail.dxf");
			if(male_key == 1)
			{
				rotate([-90,180,0]) translate([-20,length-0.1,6])
				{
					linear_extrude(1.5) scale([0.1,0.1,0.1]) import_dxf("dinkey.dxf");
				}
			}
		}

		punch_mounting_holes(female_key*0.25);
	
	};

	if(female_key == 1)
	{
		rotate([90,0,0]) translate([15,-0.1,-10])
		{
			linear_extrude(10) scale([0.11,0.11,0.11]) import_dxf("dinkey.dxf");
		}
	};
};
