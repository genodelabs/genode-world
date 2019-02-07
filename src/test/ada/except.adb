with GNAT.IO; use GNAT.IO;

package body Except
is
   procedure Do_Something
   is
   begin
      raise Test_Exception;
   end Do_Something;

   procedure Do_Sth_2
   is
   begin
      begin
         Do_Something;
      exception
         when others => Put_Line ("Exception caught (inner)");
      end;
   end Do_Sth_2;

end Except;
