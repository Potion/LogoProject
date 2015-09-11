//
//  CinderOpenCV.h
//  LogoProject
//
//  Extremely pared-down version of CinderOpenCV.h found in OpenCV Cinderblock
//
//

#pragma once
#include "opencv2/core/core.hpp"
#include "cinder/Cinder.h"
#include "cinder/ImageIo.h"

namespace cinder {
    
    class ImageSourceCvMat : public ImageSource {
    public:
        ImageSourceCvMat( const cv::Mat &mat )
        : ImageSource()
        {
            mWidth = mat.cols;
            mHeight = mat.rows;
            //  removed if-statement from original code
            //  following works only because mat.channels() == 1
            setColorModel( ImageIo::CM_GRAY );
            setChannelOrder( ImageIo::Y );
            
            //  removed switch case from original code
            // do the following b/c mat.depth() == CV_8U (0)
            setDataType( ImageIo::UINT8 );
            
            mRowBytes = mat.step;
            mData = reinterpret_cast<const uint8_t*>( mat.data );
        }
        
        void load( ImageTargetRef target ) {
            // get a pointer to the ImageSource function appropriate for handling our data configuration
            ImageSource::RowFunc func = setupRowFunc( target );
            
            const uint8_t *data = mData;
            for( int32_t row = 0; row < mHeight; ++row ) {
                ((*this).*func)( target, row, data );
                data += mRowBytes;
            }
        }
        
        const uint8_t		*mData;
        int32_t				mRowBytes;
    };
    
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    inline ImageSourceRef fromOcv( cv::Mat &mat )
    {
        return ImageSourceRef( new ImageSourceCvMat( mat ) );
    }
 
} // namespace cinder
