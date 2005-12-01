/*
******************************************************************************
*                                                                            *
* Copyright (C) 2003-2005, International Business Machines                   *
*                Corporation and others. All Rights Reserved.                *
*                                                                            *
******************************************************************************
*   file name:  ulocdata.c
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2003Oct21
*   created by: Ram Viswanadha,John Emmons
*/

#include "cmemory.h"
#include "unicode/ustring.h"
#include "unicode/ulocdata.h"

#define MEASUREMENT_SYSTEM  "MeasurementSystem"
#define PAPER_SIZE          "PaperSize"

/** A locale data object.
 *  For usage in C programs.
 *  @draft ICU 3.4
 */
typedef struct ULocaleData {
    /**
     * Controls the "No Substitute" behavior of this locale data object
     */
    UBool noSubstitute;

    /**
     * Pointer to the resource bundle associated with this locale data object
     */
    UResourceBundle *bundle;
} ULocaleData;

U_CAPI ULocaleData* U_EXPORT2
ulocdata_open(const char *localeID, UErrorCode *status)
{
   ULocaleData *uld;
 
   if (U_FAILURE(*status)) {
       return NULL;
   }

   uld = (ULocaleData *)uprv_malloc(sizeof(ULocaleData));
   if (uld == NULL) {
      *status = U_MEMORY_ALLOCATION_ERROR;
      return(NULL);
   }

    
   uld->noSubstitute = FALSE;
   uld->bundle = ures_open(NULL, localeID, status);

   if (U_FAILURE(*status)) {
      uprv_free(uld);
      return NULL;
   }

   return uld;
}

U_CAPI void U_EXPORT2
ulocdata_close(ULocaleData *uld)
{
    if ( uld != NULL ) {
       ures_close(uld->bundle);
       uprv_free(uld);
    } 
}

U_CAPI void U_EXPORT2
ulocdata_setNoSubstitute(ULocaleData *uld, UBool setting)
{
   uld->noSubstitute = setting;
}

U_CAPI UBool U_EXPORT2
ulocdata_getNoSubstitute(ULocaleData *uld)
{
   return uld->noSubstitute;
}

U_CAPI USet* U_EXPORT2
ulocdata_getExemplarSet(ULocaleData *uld, USet *fillIn, 
                        uint32_t options, ULocaleDataExemplarSetType extype, UErrorCode *status){
    
    const char* exemplarSetTypes[] = { "ExemplarCharacters", "AuxExemplarCharacters" };
    const UChar *exemplarChars = NULL;
    int32_t len = 0;
    UErrorCode localStatus = U_ZERO_ERROR;

    if (U_FAILURE(*status))
        return NULL;
    
    exemplarChars = ures_getStringByKey(uld->bundle, exemplarSetTypes[extype], &len, &localStatus);
    if ( (localStatus == U_USING_DEFAULT_WARNING) && uld->noSubstitute ) {
        localStatus = U_MISSING_RESOURCE_ERROR;
    }
       
    if (localStatus != U_ZERO_ERROR) {
        *status = localStatus;
    }
    
    if (U_FAILURE(*status))
        return NULL;
    
    if(fillIn != NULL)
        uset_applyPattern(fillIn, exemplarChars, len, 
                          USET_IGNORE_SPACE | options, status);
    else
        fillIn = uset_openPatternOptions(exemplarChars, len,
                                         USET_IGNORE_SPACE | options, status);
    
    return fillIn;

}

U_CAPI int32_t U_EXPORT2
ulocdata_getDelimiter(ULocaleData *uld, ULocaleDataDelimiterType type, 
                      UChar *result, int32_t resultLength, UErrorCode *status){

    const char* delimiterKeys[] =  { "quotationStart",
                                     "quotationEnd",
                                     "alternateQuotationStart",
                                     "alternateQuotationEnd" };

 
    UResourceBundle *delimiterBundle;
    int32_t len = 0;
    const UChar *delimiter = NULL;
    UErrorCode localStatus = U_ZERO_ERROR;

    if (U_FAILURE(*status))
        return 0;

    delimiterBundle = ures_getByKey(uld->bundle, "delimiters", NULL, &localStatus);

    if ( (localStatus == U_USING_DEFAULT_WARNING) && uld->noSubstitute ) {
        localStatus = U_MISSING_RESOURCE_ERROR;
    }

    if (localStatus != U_ZERO_ERROR) {
        *status = localStatus;
    }

    if (U_FAILURE(*status)){
        ures_close(delimiterBundle);
        return 0;
    }

    delimiter = ures_getStringByKey(delimiterBundle, delimiterKeys[type], &len, &localStatus);
    ures_close(delimiterBundle);

    if ( (localStatus == U_USING_DEFAULT_WARNING) && uld->noSubstitute ) {
        localStatus = U_MISSING_RESOURCE_ERROR;
    }

    if (localStatus != U_ZERO_ERROR) {
        *status = localStatus;
    }
    
    if (U_FAILURE(*status)){
        return 0;
    }
        
    u_strncpy(result,delimiter,resultLength);
    return len;
}

U_CAPI UMeasurementSystem U_EXPORT2
ulocdata_getMeasurementSystem(const char *localeID, UErrorCode *status){
    
    UResourceBundle* bundle=NULL;
    UResourceBundle* measurement=NULL;
    UMeasurementSystem system = UMS_LIMIT; 
    
    if(status == NULL || U_FAILURE(*status)){
        return system;
    }
    
    bundle = ures_open(NULL, localeID, status);

    measurement = ures_getByKey(bundle, MEASUREMENT_SYSTEM, NULL, status);

    system = (UMeasurementSystem) ures_getInt(measurement, status);

    ures_close(bundle);
    ures_close(measurement);

    return system;

}

U_CAPI void U_EXPORT2
ulocdata_getPaperSize(const char* localeID, int32_t *height, int32_t *width, UErrorCode *status){
    UResourceBundle* bundle=NULL;
    UResourceBundle* paperSizeBundle = NULL;
    const int32_t* paperSize=NULL;
    int32_t len = 0;

    if(status == NULL || U_FAILURE(*status)){
        return;
    }
    
    bundle = ures_open(NULL, localeID, status);
    paperSizeBundle = ures_getByKey(bundle, PAPER_SIZE, NULL, status);
    paperSize = ures_getIntVector(paperSizeBundle, &len,  status);
    
    if(U_SUCCESS(*status)){
        if(len < 2){
            *status = U_INTERNAL_PROGRAM_ERROR;
        }else{
            *height = paperSize[0];
            *width  = paperSize[1];
        }
    }
    
    ures_close(bundle);
    ures_close(paperSizeBundle);
    
}
