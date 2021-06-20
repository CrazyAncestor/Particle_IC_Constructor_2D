#include "Particle_IC_Constructor_2D.h"

//-------------------------------------------------------------------------------------------------------
// Function    :  Aux_CountRow
// Description :  Count the total number of data rows in the target file
//
// Note        :  1. Empty lines and lines starting with the comment symbol will be skipped
//                   --> The comment symbol is defined by COMMENT_SYM
//
// Parameter   :  filename : filename of the target table
//
// Return      :  Total number of matched rows
//-------------------------------------------------------------------------------------------------------
int Aux_CountRow( const char *filename )
{
   fstream file;
   file.open(filename,ios::in);

   int row=0;
   string line;
   if(!file){
      cout<<"Failed to open file:"<<filename<<endl;
   }
   else{
      do{
         getline(file,line);
         row++;
      }while(!file.eof());
   }
   file.close();

   return row;

} // FUNCTION : Aux_CountRow

//-------------------------------------------------------------------------------------------------------
// Function    :  Aux_CountRow
// Description :  Count the total number of data rows in the target file
//
// Note        :  1. Empty lines and lines starting with the comment symbol will be skipped
//                   --> The comment symbol is defined by COMMENT_SYM
//
// Parameter   :  filename : filename of the target table
//
// Return      :  Total number of matched rows
//-------------------------------------------------------------------------------------------------------
int Aux_Countcolumn( const char *filename )
{
   fstream file;
   file.open(filename,ios::in);

   int column=0;
   string line;
   if(!file){
      cout<<"Failed to open file:"<<filename<<endl;
   }
   else{
      getline(file,line,'\n');
      istringstream templine(line); // string 轉換成 stream
      while(!templine.eof()){
         getline(templine,line,' ');
         column++;
      };
   }
   file.close();
   
   return column;

} // FUNCTION : Aux_CountRow

//-------------------------------------------------------------------------------------------------------
// Function    :  LoadTable
// Description :  Load the target columns from the table
//
// Note        :  1. Overloaded with different types
//                2. Put the target columns in "TCol[]", which must be sorted into ascending numerical order
//                   in advance
//                3. Allocate memory for the pointer "Data" if AllocMem == true
//                   --> Must be freed manually
//                4. Delimiter characters for strtok() are defined by DELIMITER
//
// Parameter   :  Data        : Pointer to be allocated (if AllocMem == true) and to store the data
//                              --> call-by-reference
//                filename    : filename of the target table
//                NCol_Target : Total number of target columns
//                TCol        : Target columns (must be sorted into ascending numerical order in advance)
//                AllocMem    : true/false --> allocate/do not allocate memory for the pointer "Data"
//
// Return      :  Total number of matched rows
//-------------------------------------------------------------------------------------------------------

int LoadTable( double  *&Data, const char *filename, const int NCol_Target, const int TCol[],
                   const bool AllocMem )
{

// count the number of rows
   const int nrow = Aux_CountRow( filename );
   const int ncol = Aux_Countcolumn(filename);

// allocate memory
   if ( AllocMem )   Data = new double [NCol_Target*nrow];

// load data
   fstream file;
   file.open(filename,ios::in);

   string line;
   if(!file){
      cout<<"Failed to open file:"<<filename<<endl;
   }
   else{
      for(int row=0;row<nrow;row++){
         getline(file,line,'\n');
         istringstream templine(line); // string 轉換成 stream
         int col_targ = 0;
         for(int col=0;col<ncol;col++){
            if(col_targ>=NCol_Target)break;
            getline(templine,line,' ');
            if(col ==TCol[col_targ]){
               Data[NCol_Target*row+col_targ]=atof(line.c_str());
               col_targ++;
            }
         }
      }

   }

   file.close();
   return nrow;

} // FUNCTION : LoadTable

//-------------------------------------------------------------------------------------------------------
// Function    :  BinarySearch
// Description :  Use binary search to locate the position of an input number in a sorted array
//
// Note        :  1. "Array" must be sorted in advance in ascending numerical order
//                2. If there are multiple elements matching Key, the return index can be any of them
//                3. An overloaded function for the "long" Array and Key is also created
//                4. Overloaded with different types
//                   --> Explicit template instantiation is put in the end of this file
//
// Return      :     match --> array index
//                no match --> -1
//
// Parameter   :  Array : Sorted look-up integer array (in ascending numerical order)
//                Min   : Minimum array index for searching
//                Max   : Maximum array index for searching
//                Key   : Integer number to search for
//-------------------------------------------------------------------------------------------------------

int BinarySearch( const double  Array[], int Min, int Max, const double Key )
{

   int Mid = 0;

   while ( Min <= Max )
   {
      Mid = ( Min + Max ) / 2;

      if      ( Array[Mid] > Key )  Max = Mid-1;
      else if ( Array[Mid] < Key )  Min = Mid+1;
      else                          return Mid;
   }

   return Mid;

} // FUNCTION : BinarySearch

//-------------------------------------------------------------------------------------------------------
// Function    :  Interpolation
// Description :  Assuming y=y(x), return the interpolated value of y for a given point x
//
// Note        :  1. Interpolation table Table_x must be sorted into ascending numerical order in advance
//                2. Target point x must lie in the range Table_x[0] <= x < Table_x[N-1]
//                   --> Otherwise the function returns NULL_REAL
//                3. Currently the function only supports linear interpolation
//                4. Overloaded with different types
//                5. Explicit template instantiation is put in the end of this file
//
// Parameter   :  N        : Number of elements in the interpolation tables Table_x and Table_y
//                           --> Must be >= 2
//                Table_x  : Interpolation table x
//                Table_y  : Interpolation table y
//                x        : Target point x for interpolation
//
// Return      :  y(x)      if x lies in the range Table_x[0] <= x < Table_x[N-1]
//                NULL_REAL if x lies outside the above range
//-------------------------------------------------------------------------------------------------------

double Interpolation( const int N, const double Table_x[], const double Table_y[], const double x )
{
// binary search
   int IdxL, IdxR;
   double   xL, xR, yL, yR, y;

   IdxL = BinarySearch( Table_x, 0, N-1, x );
   

   IdxR = IdxL + 1;
   xL   = Table_x[IdxL];
   xR   = Table_x[IdxR];
   yL   = Table_y[IdxL];
   yR   = Table_y[IdxR];


// linear interpolation
   y = yL + (yR-yL)/(xR-xL)*(x-xL);

   return y;

} // FUNCTION : Interpolation
