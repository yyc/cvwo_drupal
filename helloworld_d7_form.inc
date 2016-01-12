<?php

namespace CVWO\HelloWorld {

	require_once drupal_get_path("module", MODULE_NAME)."/helloworld_d7_api.inc";

	/**
	 * Displays the form for adding/viewing/editing details of the user record
	 */
	function helloworld_form($form, &$form_state, $page_mode = 'add', $helloworld_id = null) {
		$form = array();

		if ($helloworld_id != null && $page_mode != 'add') {
			// we store the helloworld_id here for future reference (when updating/deleting later)
			$form['helloworld_id'] = array(
				'#type' => 'hidden',
				'#value' => $helloworld_id,
			);
			if (!load_user($form_state, $helloworld_id)) {
				drupal_set_message('Failed to load user details');
				drupal_goto(MODULE_NAME);
			}
		}
		
		if (isset($form_state['values'])) {
			$values = $form_state['values'];
		} else {
			$values = array(
				'name' => '',
				'email' => '',
				'comment' => '',
			);
		}
		
		if ($page_mode == 'view') {
			$disabled = TRUE;
		} else {
			$disabled = FALSE;
		}
		
		$form['name'] = array(
			'#type' => 'textfield',
			'#title' => t('Name'),
			'#default_value' => $values['name'],
			'#disabled' => $disabled,
			'#size' => 32,
			'#required' => TRUE,
		);
		
		$form['email'] = array(
			'#type' => 'textfield',
			'#title' => t('Email'),
			'#default_value' => $values['email'],
			'#disabled' => $disabled,
			'#size' => 64,
			'#required' => TRUE,
		);
		
		$form['comment'] = array(
			'#type' => 'textarea',
			'#title' => t('Comments'),
			'#disabled' => $disabled,
			'#default_value' => $values['comment'],
			'#rows' => 3,
			'#required' => FALSE,
		);
		
		// We create the buttons based on the page mode
		if ($page_mode == 'add') {
			$form['add'] = array(
				'#type' => 'submit',
				'#value' => t('Add'),
			);
		} else if ($page_mode == 'edit') {
			$form['edit'] = array(
				'#type' => 'submit',
				'#value' => t('Edit'),
			);
			
			drupal_add_js(drupal_get_path("module", MODULE_NAME)."/helloworld_d7.js");

			// check whether user has access to delete first before creating such a button
			if (user_access(DELETE_USER_PERM)) {
				$form['delete'] = array(
					'#type' => 'submit',
					'#value' => t('Delete'),
					'#attributes' => array('onclick' => 'return confirm_helloworld_form_delete()'),
				);
			}
		}
	  
		return $form;
	}


    /**
     * Displays the form for mass-adding a user record
     */
	function helloworld_massedit_form($form, &$form_state) {
		$form = array();

/*
		if ($helloworld_id != null && $page_mode != 'add') {
			// we store the helloworld_id here for future reference (when updating/deleting later)
			$form['helloworld_id'] = array(
				'#type' => 'hidden',
				'#value' => $helloworld_id,
			);
			if (!load_user($form_state, $helloworld_id)) {
				drupal_set_message('Failed to load user details');
				drupal_goto(MODULE_NAME);
			}
		}
*/
		
		if (isset($form_state['values'])) {
			$values = $form_state['values'];
		} else {
			$values = array(
    			'csv' => ''
			);
		}

		$form['csv'] = array(
			'#type' => 'textarea',
			'#title' => t('Users'),
			'#default_value' => $values['csv'],
			'#required' => TRUE,
		);

		
			$form['add'] = array(
				'#type' => 'submit',
				'#value' => t('Add'),
			);
		return $form;
	}

	/**
	 * Implements form_validate
	 */
	function helloworld_form_validate($form, &$form_state){
		// valid_email_address is from drupal api
		// validation function checks whether the email is valid before inserting/updating/deleting
		if (!valid_email_address($form_state['values']['email'])){
			form_set_error('email', t('Please enter a valid email address.'));
		}
	}

	/**
	 * Implements form_submit
	 */
	function helloworld_form_submit($form, &$form_state){
		$button_name = $form_state['values']['op'];
		$error = FALSE;

		switch($button_name) {
			case 'Add': 
				if (add_user($form_state['values'])) {
					drupal_set_message(t('%name has been successfully added', array('%name' => $form_state['values']['name'])));
				} else {
					drupal_set_message(t('An error was encountered while adding %name', array('%name' => $form_state['values']['name'])));
					$error = TRUE;
				}
				break;
			case 'Edit':
				if (update_user($form_state['values'])) {
					drupal_set_message(t('%name has been successfully updated', array('%name' => $form_state['values']['name'])));
				} else {
					drupal_set_message(t('An error was encountered while trying to update %name', array('%name' => $form_state['values']['name'])));
					$error = TRUE;
				}
				break;
			case 'Delete':
				if (delete_user($form_state['values']['helloworld_id'])) {
					drupal_set_message(t('%name has been successfully deleted', array('%name' => $form_state['values']['name'])));
				} else {
					drupal_set_message(t('An error was encountered while trying to delete %name', array('%name' => $form_state['values']['name'])));
					$error = TRUE;
				}
				break;
		}
		
		// if we encountered an error, we keep the form as it is
		// else we redirect back to home page
		if ($error) {
			$form_state['rebuild'] = TRUE;
		} else {
			$form_state['redirect'] = MODULE_NAME;
		}
	}

	function helloworld_massedit_form_validate($form, &$form_state){
		if(!isset($form['csv']) || $form['csv']['#value'] == ''){
    		form_set_error("csv", "no values were entered");
		} else{
            $records = str_getcsv($form['csv']['#value'], "\n");
            $rejected = [];
            $users = [];
            foreach($records as &$row){
                $cols = str_getcsv($row);
                if(count($cols) == 2 && valid_email_address($cols[1])){
                    $users[] = array('name' => $cols[0], 'email' => $cols[1], 'comment' => '');
                } else{
                    $rejected[] = $row;
                }
            }
            if(count($rejected) > 0){
                $form_state['values']['csv'] = $rejected;
                store_multiple_users($users, $form_state);
        		drupal_set_message("The following lines are invalid comma-separated values or have invalid email addresses.", "error");
            } else{
                $form_state['values']['csv'] = array();
    		}
		}
        
	}

    function helloworld_massedit_form_submit($form, &$form_state){
        store_multiple_users($users, $form_state);
    }
    
    function store_multiple_users($users, &$form_state){
        $successes = [];
        $errors = [];
        foreach($users as $user){
			if (add_user($user)) {
                $successes[] = $user['name'];
			} else {
    			$errors = $user;
			}
        }
        if(count($successes) > 0){
            drupal_set_message(t("The following users were successfully stored: %names.", array('%names' => implode(', ', $successes))));
        }
        if(count($errors) > 0){
            $error_names = array_map(function($error){
                return $error[0];
            }, $errors);
			drupal_set_message(t('An error was encountered while adding %names', array('%names' => implode(', ', $error_names))), 'error');
			$form_state['values']['csv'] = array_merge($form_state['values']['csv'], array_map(function($error){
    			return format_string("%name,%email", array('%name' => $error['name'], '%email'=> $error['email']));
			}));
		}
		if(count($form_state['values']['csv']) > 0){
    		$form_state['input']['csv'] = implode("\n", $form_state['values']['csv']);
    		$form_state['rebuild'] = True;
		}
    }
    
	/**
	 * Function implements the Helloworld admin settings form
	 */ 
	 function helloworld_admin() {
		$form[PAGELIMIT] = array(
			'#type' => 'textfield',
			'#title' => t('Maximum number of records in a page'),
			'#default_value' => variable_get(PAGELIMIT),
			'#size' => 2,
			'#maxlength' => 2,
			'#description' => t("The maximum number of records to display in the user records listing."),
			'#required' => TRUE,
		);
		
		$form[AUTOCOMPLETELIMIT] = array(
			'#type' => 'textfield',
			'#title' => t('Maximum number of records in autocomplete'),
			'#default_value' => variable_get(AUTOCOMPLETELIMIT),
			'#size' => 2,
			'#maxlength' => 2,
			'#description' => t("The maximum number of records to display under autocomplete function."),
			'#required' => TRUE,
		);
		
		return system_settings_form($form);
	 }
	 
	 /**
	 * Form validation handler for helloworld_admin().
	 *
	 * @see helloworld_admin()
	 */
	function helloworld_admin_validate($form, &$form_state) {
	  $helloworld_pagelimit = $form_state['values'][PAGELIMIT];
	  $helloworld_autocompletelimit = $form_state['values'][AUTOCOMPLETELIMIT];
	  
	  if (!is_numeric($helloworld_pagelimit)) {
	    form_set_error('helloworld_pagelimit', t('You must enter a number for the maximum number of records in a page.'));
	  } else if ($helloworld_pagelimit <= 0) {
	    form_set_error('helloworld_pagelimit', t('Maximum number of records in a page must be positive.'));
	  }
	 
	  if (!is_numeric($helloworld_autocompletelimit)) {
	    form_set_error('helloworld_autocompletelimit', t('You must enter a number for the maximum number of records displayed in autocomplete function.'));
	  } else if ($helloworld_autocompletelimit <= 0) {
	    form_set_error('helloworld_autocompletelimit', t('Maximum number of records in autocomplete function must be positive.'));
	  } 
	 
	}
}
