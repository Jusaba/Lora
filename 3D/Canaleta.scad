//Bisel Izquierda 
Bisel = 2; // [ 0: Sin Bisel, 1: Bisel Izquierda -> Derecha, 2: Bisel Derecha -> Izquierda ] 
//Length of DIN Rail to print (mm)
length = 110;//[30:200]

//Number of holes on the rail
holes_per_segment = 2; //[0:10]

//Diameter of mounting holes (mm)
hole_diameter = 3; //[0.5:5]




module punch_mounting_holes(adjustment)
{
    echo ("Hola");
	for(j = [1:holes_per_segment])
	{
        echo (j);
        echo ((j)*(length-adjustment)/(holes_per_segment + 1));
		translate([9.5,2,(j)*(length-adjustment)/(holes_per_segment + 1)]) 
			rotate([90,0,0]) 
				cylinder(r=hole_diameter/2, h=3, $fn=8);
	}
}

module punch_mounting_bisel ()
{
    if ( Bisel == 1 )
    {    
        translate([0,10,length])
            rotate([90,0,0]) 
            cylinder(r=19, h=10, $fn=4);
    }
    if ( Bisel == 2 )
    {    
        translate([19,10,length])
            rotate([90,0,0]) 
            cylinder(r=19, h=10, $fn=4);
    }    
}


difference()
{
    difference()
        difference()
        {
            linear_extrude(length)import("PerfilCanaleta.dxf");
            if ( holes_per_segment > 0 )
            {    
                punch_mounting_holes(9.4);
            }    
        }    
        if (Bisel > 0)
        {    
            punch_mounting_bisel();
        }
    
};
//punch_mounting_bisel ();
